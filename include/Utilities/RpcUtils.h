#pragma once

#include <string>
#include <map>
#include <ArduinoJson.h>
#include "System_Utils.h"

namespace
{
    std::map<std::string, RpcFunction> _rpcMap;
};

namespace RpcModule::Utilities
{
    using RpcFunction = void (*)(JsonDocument &doc);

    void RegisterRpc(std::string name, RpcFunction function)
    {
        _rpcMap[name] = function;
    }

    void UnregisterRpc(std::string name)
    {
        _rpcMap.erase(name);
    }

    void CallRpc(std::string name, JsonDocument &doc)
    {
        if (_rpcMap.find(name) != _rpcMap.end())
        {
            _rpcMap[name](doc);
        }
    }
};


 