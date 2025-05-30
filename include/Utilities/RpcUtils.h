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

        static RpcReturnCode CallRpc(std::string name, JsonDocument &doc)
        {
            if (_rpcMap.find(name) != _rpcMap.end())
            {
                #if DEBUG == 1
                Serial.print("Calling function ");
                Serial.println(name.c_str());
                #endif

                _rpcMap[name](doc);
                return RpcReturnCode::RPC_SUCCESS;
            }

            #if DEBUG == 1
            Serial.print("Function ");
            Serial.print(name.c_str());
            Serial.println(" not registered");
            #endif
            return RpcReturnCode::RPC_FUNCTION_NOT_REGISTERED;
        }

        static int AddRpcChannel(size_t bufferMaxSize, RpcRequestSource pollFunctionPointer, RpcReplyDestination replyFunctionPointer)
        {
            int channelID = _CurrentChannelID;
            _CurrentChannelID++;

            auto result = RpcChannels().emplace(channelID, RpcChannel(channelID, bufferMaxSize, pollFunctionPointer, replyFunctionPointer));

            if (result.second)
            {
                #if DEBUG == 1
                Serial.print("Added channel ");
                Serial.println(channelID);
                #endif

                return channelID;
            }
            else
            {
                #if DEBUG == 1
                Serial.print("Failed to add channel ");
                Serial.println(channelID);
                #endif
                return -1;
            }
        }

        static void RemoveRpcChannel(int channelID)
        {
            if (RpcChannels().find(channelID) != RpcChannels().end())
            {
                RpcChannels().erase(channelID);
            }
        }

        static void EnableRpcChannel(int channelID)
        {
            if (RpcChannels().find(channelID) != RpcChannels().end())
            {
                #if DEBUG == 1
                Serial.print("Enabling channel "); 
                Serial.println(channelID);
                #endif
                RpcChannels()[channelID].IsActive = true;
            }
            #if DEBUG == 1
            else
            {
                Serial.print("Failed to enable channel "); 
                Serial.println(channelID);
            }
            #endif
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


 