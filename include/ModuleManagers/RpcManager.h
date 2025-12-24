#pragma once

#include <Arduino.h>
#include <unordered_map>
#include "RpcUtils.h"
#include "RpcChannel.h"
#include "System_Utils.h"
#include "VersionUtils.h"
#include "ESPAsyncWebServer.h"
#include <string>

namespace RpcModule
{

    namespace
    {
        const size_t RPC_THREAD_DELAY_MS = 100;
    }

    class Manager
    {
    public:
        Manager() {}

        ~Manager() {}

        void Init(int taskPriority, size_t taskCore = 0)
        {
            System_Utils::registerTask(BoundRpcTask,
            "RpcLoop",
            8192,
            this,
            taskPriority, 
            taskCore);
        }

        int RegisterRpcChannel(RpcRequestSource pollFunctionPointer, RpcReplyDestination replyFunctionPointer, size_t bufferMaxSize = 512)
        {
            if (pollFunctionPointer == nullptr)
            {
                return -1;
            }

            return Utilities::AddRpcChannel(bufferMaxSize, pollFunctionPointer, replyFunctionPointer);
        }

        void ProcessRpcChannels()
        {
            while (true)
            {
                for (auto &channel : Utilities::RpcChannels())
                {
                    if (channel.second.IsActive)
                    {
                        ESP_LOGV(TAG, "Polling channel %d", channel.second.ChannelID);

                        DynamicJsonDocument rpcPayload(channel.second.BufferMaxSize);
                        auto channelID = channel.second.ChannelID;
                        if (!channel.second.PollFunctionPointer(channelID, rpcPayload))
                        {
                            continue;
                        }

                        std::string buf;
                        serializeJson(rpcPayload, buf);
                        ESP_LOGI(TAG, "MsgPack found on channel %d: %s", channelID, buf.c_str());

                        if (rpcPayload.containsKey(Utilities::RPC_FUNCTION_NAME_FIELD())) 
                        {
                            auto result = Utilities::CallRpc(rpcPayload[Utilities::RPC_FUNCTION_NAME_FIELD()].as<std::string>(), rpcPayload);

                            if (result != RpcReturnCode::RPC_SUCCESS)
                            {
                                rpcPayload.clear();
                                rpcPayload[Utilities::RPC_RETURN_CODE_FIELD()] = (int)result;
                            }

                            if (channel.second.ReturnSupported)
                            {
                                channel.second.ReplyFunctionPointer(channelID, rpcPayload);
                            }
                        }
                    }
                }

                vTaskDelay(pdMS_TO_TICKS(RPC_THREAD_DELAY_MS));
            }
        }

        void RegisterSerialRpc() 
        {
            RpcRequestSource pollFunctionPointer = [](int channelID, JsonDocument &payload) -> bool 
            {
                if (Serial.available() > 0) 
                {
                    auto result = deserializeMsgPack(payload, Serial);

                    if (result != DeserializationError::Ok)
                    {
                        return false;
                    }

                    return true;
                }

                return false;
            };

            RpcReplyDestination replyFunctionPointer = [](int channelID, JsonDocument &payload) 
            {
                if (Serial.availableForWrite() > 0) 
                {
                    serializeMsgPack(payload, Serial);
                }
            };

            _serialRpcChannelID = Utilities::AddRpcChannel(512, pollFunctionPointer, replyFunctionPointer);
            // Utilities::EnableRpcChannel(_serialRpcChannelID);
        }

        void RegisterWebServerRpc(AsyncWebServer &server)
        {
            std::function<void(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)> onRpcCallbackBody;
            std::function<void(AsyncWebServerRequest *request)> onRpcCallback = [](AsyncWebServerRequest *request) {  };

            onRpcCallbackBody = [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
                ESP_LOGI(TAG, "Received request body");

                // Deserialize MessagePack to JSON
                DynamicJsonDocument doc(16000);
                DeserializationError error = deserializeMsgPack(doc, data, len);
                
                if (error) {
                    request->send(400, "text/plain", "Invalid MessagePack data");
                    return;
                }

                if (!doc.as<JsonObject>().containsKey(RpcModule::Utilities::RPC_FUNCTION_NAME_FIELD()))
                {
                    request->send(404, "text/plain", "Packet does not contain function name");
                    return;
                }

                auto returnCode = RpcModule::Utilities::CallRpc(doc[RpcModule::Utilities::RPC_FUNCTION_NAME_FIELD()].as<std::string>(), doc);

                switch (returnCode)
                {
                // case RpcModule::RpcReturnCode::RPC_SUCCESS:
                //     request->send(200, "text/plain", "Success");
                //     break;
                case RpcModule::RpcReturnCode::RPC_SUCCESS:
                    {
                        size_t packedSize = measureMsgPack(doc);
                        uint8_t *responseBuffer = new uint8_t[packedSize];  // Allocate memory
                        serializeMsgPack(doc, responseBuffer, packedSize);

                        AsyncWebServerResponse *response = request->beginResponse(
                            "application/msgpack", packedSize, 
                            [responseBuffer, packedSize](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                                size_t copyLen = (index + maxLen > packedSize) ? (packedSize - index) : maxLen;
                                memcpy(buffer, responseBuffer + index, copyLen);
                                if (index + maxLen >= packedSize) {
                                    delete[] responseBuffer;  
                                }
                                return copyLen;
                            }
                        );
                        response->addHeader("Content-Type", "application/msgpack");
                        request->send(response);
                    }
                    break;
                case RpcModule::RpcReturnCode::RPC_FUNCTION_NOT_REGISTERED:
                    request->send(400, "text/plain", "Function not registered");
                    break;
                case RpcModule::RpcReturnCode::RPC_FUNCTION_ERROR:
                    request->send(500, "text/plain", "Function error");
                    break;
                default:
                    request->send(500, "text/plain", "Unknown error");
                    break;
                }
            };

            server.on(
                "/rpc",
                (WebRequestMethodComposite)HTTP_POST,
                onRpcCallback,
                nullptr,
                onRpcCallbackBody
            );

            server.on(
                "/ping",
                (WebRequestMethodComposite)HTTP_GET,
                [](AsyncWebServerRequest *request) { request->send(200, "text/plain", "pong"); },
                nullptr,
                nullptr
            );
            
            server.on(
                "/",
                (WebRequestMethodComposite)HTTP_GET,
                [](AsyncWebServerRequest *request) 
                { 
                    StaticJsonDocument<256> doc;
                    doc["DeviceName"] = System_Utils::DeviceName;
                    doc["DeviceID"] = System_Utils::DeviceID;
                    doc["FirmwareVersion"] = FIRMWARE_VERSION_STRING;
                    #ifdef HARDWARE_VERSION
                    doc["HardwareVersion"] = HARDWARE_VERSION;
                    #else
                    doc["HardwareVersion"] = 0;
                    #endif
                    std::string jsonReturn;
                    serializeJson(doc, jsonReturn);
                    request->send(200, "json", jsonReturn.c_str());   
                },
                nullptr,
                nullptr
            );
        }

    protected:

        int _serialRpcChannelID = -1;

        static void BoundRpcTask(void *pvParameters)
        {
            ESP_LOGI(TAG, "Rpc loop started");

            Manager *manager = (Manager *)pvParameters;
            manager->RegisterSerialRpc();
            manager->ProcessRpcChannels();

            ESP_LOGI(TAG, "Rpc loop exited");

            vTaskDelete(NULL);
        }
    };
}