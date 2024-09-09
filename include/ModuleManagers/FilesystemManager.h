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
        FilesystemUtils::Init();
    }
};