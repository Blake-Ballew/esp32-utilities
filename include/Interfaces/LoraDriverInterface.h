#pragma once

#include <stddef.h>
#include <stdint.h>

class LoraDriverInterface
{
public:
    virtual bool Init() = 0;
    virtual bool ReceiveMessage(uint8_t* buffer, size_t& outLen, size_t timeout) = 0;
    virtual bool SendMessage(const uint8_t* buffer, size_t len) = 0;
    virtual ~LoraDriverInterface() = default;
};
