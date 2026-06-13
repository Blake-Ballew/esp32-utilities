#pragma once

#include <stddef.h>
#include <stdint.h>

class LoraDriverInterface
{
public:
    virtual bool Init() = 0;
    virtual bool ReceiveMessage(uint8_t* buffer, size_t& outLen, size_t timeout) = 0;
    virtual bool SendMessage(const uint8_t* buffer, size_t len) = 0;
    virtual void RegisterOnReceive(void(*callback)(int)) = 0;
    virtual void StartReceiving() = 0;
    virtual int  PacketRssi() = 0;
    virtual bool IsChannelBusy() = 0;
    virtual ~LoraDriverInterface() = default;
};
