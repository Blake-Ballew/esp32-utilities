#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

namespace RpcModule
{
    // using RpcRequestSource = void (*)(uint8_t channelID, JsonDocument &payload);
    // using RpcReplyDestination = void (*)(uint8_t channelID, JsonDocument &payload);

    // A function that looks for a message pack and returns true if one is found
    using RpcRequestSource = std::function<bool(int, JsonDocument &)>;

    // 
    using RpcReplyDestination = std::function<void(int, JsonDocument &)>;

    struct RpcChannel
    {
        int ChannelID;
        bool IsActive;
        size_t BufferMaxSize;
        RpcRequestSource PollFunctionPointer;
        RpcReplyDestination ReplyFunctionPointer;

        bool ReturnSupported = true;

        RpcChannel() : ChannelID(-1), IsActive(false), BufferMaxSize(0) {}

        RpcChannel(int channelID, size_t bufferMaxSize, RpcRequestSource pollFunctionPointer, RpcReplyDestination replyFunctionPointer)
            : ChannelID(channelID), IsActive(false), BufferMaxSize(bufferMaxSize), PollFunctionPointer(pollFunctionPointer), ReplyFunctionPointer(replyFunctionPointer) {}

        // Copy constructor
        // RpcChannel(const RpcChannel &other) : ChannelID(other.ChannelID), IsActive(other.IsActive), BufferMaxSize(other.BufferMaxSize), PollFunctionPointer(other.PollFunctionPointer), ReplyFunctionPointer(other.ReplyFunctionPointer) {}
    };
}