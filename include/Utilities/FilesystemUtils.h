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
    READ_ERROR = 4
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

    // Loads the settings file from the given path
    static FilesystemReturnCode LoadSettingsFile(std::string filename);

    // Settings file getter
    static JsonDocument &SettingsFile() { return _SettingsFile; }

    // print settings file to Serial
    static void PrintSettingsFile() { serializeJson(_SettingsFile, Serial); }

    // Settings file setter
    static FilesystemReturnCode WriteSettingsFile(std::string filename, JsonDocument &doc) 
    {
        auto returncode = WriteFile(filename, doc);
        if (returncode == FilesystemReturnCode::FILESYSTEM_OK && &doc != &_SettingsFile)
        {
            _SettingsFile.set(doc.as<JsonObject>());
        }
        return returncode;
    }

    static FilesystemReturnCode WriteSettingsFileToFlash()
    {
        if (_SettingsFilename.length() == 0 || _SettingsFile.isNull())
        {
            return FilesystemReturnCode::WRITE_FAILED;
        }

        _SettingsUpdated.Invoke();
        return WriteFile(_SettingsFilename, _SettingsFile);
    }

    // SettingsUpdated event handler getter
    static EventHandler &SettingsUpdated() { return _SettingsUpdated; }

protected:
    // Settings File
    static DynamicJsonDocument _SettingsFile;
    static std::string _SettingsFilename;

    // Event handler for settings file updates
    static EventHandler _SettingsUpdated;
};
