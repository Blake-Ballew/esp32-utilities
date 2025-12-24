#pragma once

#include <string>
#include <unordered_map>
#include <ArduinoJson.h>
#include "RpcChannel.h"
#include "System_Utils.h"

namespace RpcModule
{
    // using RpcFunction = void (*)(JsonDocument &doc);
    using RpcFunction = std::function<void(JsonDocument &doc)>;

    static const char *TAG = "RpcModule";

    namespace
    {
        std::unordered_map<std::string, RpcFunction> _rpcMap;

        int _CurrentChannelID = 0;

        const char *_RPC_FUNCTION_NAME_FIELD PROGMEM = "F";
        const char *_RPC_RETURN_CODE_FIELD PROGMEM = "R";
    };

    enum RpcReturnCode
    {
        RPC_SUCCESS = 0,
        RPC_SUCCESS_WITH_PAYLOAD = 1,
        RPC_FUNCTION_NOT_REGISTERED = 2,
        RPC_FUNCTION_ERROR = 3,
    };

    class Utilities
    {
    public:
        static void RegisterRpc(std::string name, RpcFunction function)
        {
            _rpcMap[name] = function;
        }

        static void UnregisterRpc(std::string name)
        {
            _rpcMap.erase(name);
        }

        static bool RpcResponseNullDestination(int channelID, JsonDocument &payload)
    {
        // Do nothing
        return true;
    }

        static RpcReturnCode CallRpc(std::string name, JsonDocument &doc)
        {
            if (_rpcMap.find(name) != _rpcMap.end())
            {
                ESP_LOGV(TAG, "Calling function %s", name.c_str());

                _rpcMap[name](doc);
                return RpcReturnCode::RPC_SUCCESS;
            }

            ESP_LOGV(TAG, "Function %s not registered", name.c_str());
            return RpcReturnCode::RPC_FUNCTION_NOT_REGISTERED;
        }

        static int AddRpcChannel(size_t bufferMaxSize, RpcRequestSource pollFunctionPointer, RpcReplyDestination replyFunctionPointer)
        {
            int channelID = _CurrentChannelID;
            _CurrentChannelID++;

            auto result = RpcChannels().emplace(channelID, RpcChannel(channelID, bufferMaxSize, pollFunctionPointer, replyFunctionPointer));

            if (result.second)
            {
                ESP_LOGV(TAG, "Added channel %d", channelID);

                return channelID;
            }
            else
            {
                ESP_LOGV(TAG, "Failed to add channel %d", channelID);
                return -1;
            }
        }

        static void RemoveRpcChannel(int channelID)
        {
            if (RpcChannels().find(channelID) != RpcChannels().end())
            {
                ESP_LOGV(TAG, "Removing channel %d", channelID);
                RpcChannels().erase(channelID);
            }
        }

        static void EnableRpcChannel(int channelID)
        {
            if (RpcChannels().find(channelID) != RpcChannels().end())
            {
                ESP_LOGV(TAG, "Enabling channel %d", channelID);
                RpcChannels()[channelID].IsActive = true;
            }
            else
            {
                ESP_LOGV(TAG, "Failed to enable channel %d", channelID);
            }
        }

        static void DisableRpcChannel(int channelID)
        {
            if (RpcChannels().find(channelID) != RpcChannels().end())
            {
                RpcChannels()[channelID].IsActive = false;
            }
        }

        static std::unordered_map<int, RpcChannel> &RpcChannels() 
        { 
            static std::unordered_map<int, RpcChannel> _RpcChannels;
            return _RpcChannels;
        }

        static std::unordered_map<std::string, RpcFunction> &RpcMap() 
        {
            static std::unordered_map<std::string, RpcFunction> _rpcMap;
            return _rpcMap;
        }

        static const char *RPC_FUNCTION_NAME_FIELD() { return _RPC_FUNCTION_NAME_FIELD; }
        static const char *RPC_RETURN_CODE_FIELD() { return _RPC_RETURN_CODE_FIELD; }
    };
};


 