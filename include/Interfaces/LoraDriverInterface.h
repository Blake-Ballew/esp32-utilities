#pragma once

#include "ArduinoJson.h"

class LoraDriverInterface
{
public:
    virtual bool Init() = 0;

    virtual bool ReceiveMessage(JsonDocument &doc, size_t timeout) = 0;
    virtual bool SendMessage(JsonDocument &doc) = 0;
};