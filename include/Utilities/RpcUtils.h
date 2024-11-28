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

    namespace
    {
        std::unordered_map<std::string, RpcFunction> _rpcMap;

        std::unordered_map<int, RpcChannel> _RpcChannels;
        int _CurrentChannelID = 0;

        const char *_RPC_FUNCTION_NAME_FIELD PROGMEM = "F";
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

        static RpcReturnCode CallRpc(std::string name, JsonDocument &doc)
        {
            if (_rpcMap.find(name) != _rpcMap.end())
            {
                _rpcMap[name](doc);
                return RpcReturnCode::RPC_SUCCESS;
            }

            return RpcReturnCode::RPC_FUNCTION_NOT_REGISTERED;
        }

        static int AddRpcChannel(size_t bufferMaxSize, RpcRequestSource pollFunctionPointer, RpcReplyDestination replyFunctionPointer)
        {
            int channelID = _CurrentChannelID;
            _CurrentChannelID++;
            _RpcChannels[channelID] = RpcChannel(channelID, bufferMaxSize, pollFunctionPointer, replyFunctionPointer);
            return channelID;
        }

        static void RemoveRpcChannel(int channelID)
        {
            if (_RpcChannels.find(channelID) != _RpcChannels.end())
            {
                _RpcChannels.erase(channelID);
            }
        }

        static void EnableRpcChannel(int channelID)
        {
            if (_RpcChannels.find(channelID) != _RpcChannels.end())
            {
                _RpcChannels[channelID].IsActive = true;
            }
        }

        static void DisableRpcChannel(int channelID)
        {
            if (_RpcChannels.find(channelID) != _RpcChannels.end())
            {
                _RpcChannels[channelID].IsActive = false;
            }
        }

        static const std::unordered_map<int, RpcChannel> &RpcChannels() { return _RpcChannels; }

        static const char *RPC_FUNCTION_NAME_FIELD() { return _RPC_FUNCTION_NAME_FIELD; }
    };
};


 