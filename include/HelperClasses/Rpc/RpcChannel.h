#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

namespace RpcModule
{
    // using RpcRequestSource = void (*)(uint8_t channelID, JsonDocument &payload);
    // using RpcReplyDestination = void (*)(uint8_t channelID, JsonDocument &payload);

    // A function that looks for a message pack and returns true if one is found
    using RpcRequestSource = std::function<bool(int channelID, JsonDocument &payload)>;

    // 
    using RpcReplyDestination = std::function<void(int channelID, JsonDocument &payload)>;

    struct RpcChannel
    {
        int ChannelID;
        bool IsActive;
        size_t BufferMaxSize;
        RpcRequestSource PollFunctionPointer;
        RpcReplyDestination ReplyFunctionPointer;

        bool ReturnSupported = true;

        RpcChannel() : ChannelID(-1), IsActive(false), BufferMaxSize(0), PollFunctionPointer(nullptr), ReplyFunctionPointer(nullptr) {}

        RpcChannel(int channelID, size_t bufferMaxSize, RpcRequestSource pollFunctionPointer, RpcReplyDestination replyFunctionPointer)
            : ChannelID(channelID), IsActive(false), BufferMaxSize(bufferMaxSize), PollFunctionPointer(pollFunctionPointer), ReplyFunctionPointer(replyFunctionPointer) {}
    };
}