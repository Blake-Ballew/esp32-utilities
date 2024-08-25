#pragma once

#include "ArduinoJson.h"
#include "System_Utils.h"
#include <StreamUtils.h>
#include <SPIFFS.h>
#include <string>

enum FilesystemReturnCode
{
    FILESYSTEM_OK = 0,
    READ_BUFFER_OVERFLOW = 1,
    FILE_NOT_FOUND = 2,
    WRITE_FAILED = 3,
};

// Static class with helper functions for interacting with the SPIFFS filesystem
// Data is stored in MessagePack format
class FilesystemUtils
{
public:
    // Initializes the SPIFFS filesystem. Formats the filesystem if it doesn't exist and reboots the device
    static void Init();

    // Reads a file from the SPIFFS filesystem into a JsonDocument
    static FilesystemReturnCode ReadFile(std::string filename, JsonDocument &doc);

    // Writes a file to the SPIFFS filesystem from a JsonDocument
    static FilesystemReturnCode WriteFile(std::string filename, JsonDocument &doc);
};
