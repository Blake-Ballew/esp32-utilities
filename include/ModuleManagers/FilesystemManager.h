#pragma once

#include "FilesystemUtils.h"

// Manager class for the SPIFFS filesystem
class FilesystemManager
{
public:
    FilesystemManager() {}
    ~FilesystemManager() {}

    void Init()
    {
        #if DEBUG == 1
        Serial.println("FilesystemManager::Init");
        #endif
        FilesystemUtils::Init();
    }
};