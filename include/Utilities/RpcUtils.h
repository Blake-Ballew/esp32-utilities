#pragma once

#include <string>
#include <map>
#include <ArduinoJson.h>
#include "System_Utils.h"

namespace RpcModule
{
    using RpcFunction = void (*)(JsonDocument &doc);

    namespace
    {
        std::map<std::string, RpcFunction> _rpcMap;
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

        static void CallRpc(std::string name, JsonDocument &doc)
        {
            if (_rpcMap.find(name) != _rpcMap.end())
            {
                _rpcMap[name](doc);
            }
        }
    };
};


 