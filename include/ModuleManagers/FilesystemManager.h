#pragma once

#include "FilesystemUtils.h"

namespace FilesystemModule
{
// Manager class for the SPIFFS filesystem
    class Manager
    {
    public:
        Manager() {}
        ~Manager() {}

        void InitializeFilesystem()
        {
            #if DEBUG == 1
            Serial.println("FilesystemManager::Init");
            #endif
            Utilities::Init();
        }
    };
}