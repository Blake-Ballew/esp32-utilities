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

        void Init(int taskPriority)
        {
            System_Utils::registerTask([] (void *pvParameters) 
            {
                Manager *manager = (Manager *)pvParameters;
                manager->RegisterSerialRpc();
                manager->ProcessRpcChannels();

                vTaskDelete(NULL);
            },
            "RpcLoop",
            4096,
            this,
            taskPriority);
        }

        void ProcessRpcChannels()
        {
            while (true)
            {
                for (auto &channel : Utilities::RpcChannels())
                {
                    if (channel.second.IsActive)
                    {
                        DynamicJsonDocument rpcPayload(channel.second.BufferMaxSize);
                        auto channelID = channel.second.ChannelID;
                        channel.second.PollFunctionPointer(channelID, rpcPayload);

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

            Utilities::AddRpcChannel(512, pollFunctionPointer, replyFunctionPointer);
        }

    protected:

    };
}