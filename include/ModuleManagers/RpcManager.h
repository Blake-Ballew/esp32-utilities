#pragma once

#include <Arduino.h>
#include <unordered_map>
#include "RpcUtils.h"
#include "RpcChannel.h"
#include "System_Utils.h"

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

        void ProcessRpcChannels()
        {
            while (true)
            {
                for (auto &channel : Utilities::RpcChannels())
                {
                    if (channel.second.IsActive)
                    {
                        #if DEBUG == 1
                        Serial.print("Polling channel ");
                        Serial.println(channel.second.ChannelID);
                        #endif

                        DynamicJsonDocument rpcPayload(channel.second.BufferMaxSize);
                        auto channelID = channel.second.ChannelID;
                        if (!channel.second.PollFunctionPointer(channelID, rpcPayload))
                        {
                            continue;
                        }

                        #if DEBUG == 1
                        Serial.print("MsgPack found on channel ");
                        Serial.print(channelID);
                        Serial.println(": ");
                        serializeJson(rpcPayload, Serial);
                        Serial.println();
                        #endif

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

    protected:

        int _serialRpcChannelID = -1;

        static void BoundRpcTask(void *pvParameters) 
        {
            #if DEBUG == 1
            Serial.println("Rpc loop started");
            #endif

            Manager *manager = (Manager *)pvParameters;
            manager->RegisterSerialRpc();
            manager->ProcessRpcChannels();

            #if DEBUG == 1
            Serial.println("Rpc loop exited");
            #endif

            vTaskDelete(NULL);
        }
    };
}