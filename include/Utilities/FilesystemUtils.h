#pragma once

#include "ArduinoJson.h"
#include "System_Utils.h"
#include <StreamUtils.h>
#include <SPIFFS.h>
#include <string>

namespace FilesystemModule
{

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
    class Utilities
    {
    public:
        // Initializes the SPIFFS filesystem. Formats the filesystem if it doesn't exist and reboots the device
        static void Init()
        {
            if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
            {
                Serial.println("SPIFFS Mount Failed. Rebooting...");
                vTaskDelay(pdMS_TO_TICKS(1000));
                ESP.restart();
            }
        }

        // Reads a file from the SPIFFS filesystem into a JsonDocument
        static FilesystemReturnCode ReadFile(std::string filename, JsonDocument &doc)
        {   
            #if DEBUG == 1
            Serial.print("Reading file: ");
            Serial.println(filename.c_str());
            #endif

            if (!SPIFFS.exists(filename.c_str()))
            {
                #if DEBUG == 1
                Serial.print("File not found: ");
                Serial.println(filename.c_str());
                #endif
                return FilesystemReturnCode::FILE_NOT_FOUND;
            }
        
            File file = SPIFFS.open(filename.c_str(), FILE_READ);
        
            if (!file)
            {
                #if DEBUG == 1
                Serial.print("Failed to open file: ");
                Serial.println(filename.c_str());
                #endif
                return FilesystemReturnCode::READ_ERROR;
            }
        
            deserializeMsgPack(doc, file); 
            file.close();
        
            if (doc.overflowed())
            {
                #if DEBUG == 1
                Serial.print("Buffer overflow while reading file: ");
                Serial.println(filename.c_str());
                #endif
                return FilesystemReturnCode::READ_BUFFER_OVERFLOW;
            }
        
            if (doc.isNull())
            {
                #if DEBUG == 1
                Serial.print("Failed to deserialize file: ");
                Serial.println(filename.c_str());
                #endif
                return FilesystemReturnCode::READ_ERROR;
            }
            else
            {
                #if DEBUG == 1
                Serial.print("File read successfully: ");
                Serial.println(filename.c_str());
                serializeJson(doc, Serial);
                Serial.println();
                #endif
            }
            return FilesystemReturnCode::FILESYSTEM_OK;
        }

        // Writes a file to the SPIFFS filesystem from a JsonDocument
        static FilesystemReturnCode WriteFile(std::string filename, JsonDocument &doc)
        {
            #if DEBUG == 1
            // Serial.print("Writing file: ");
            // Serial.println(filename.c_str());
            // serializeJson(doc, Serial);
            // Serial.println();
            #endif

            File file = SPIFFS.open(filename.c_str(), FILE_WRITE);
            if (!file)
            {
                return FilesystemReturnCode::WRITE_FAILED;
            }
        
            auto bytesWritten = serializeMsgPack(doc, file);
            file.close();
        
            if (bytesWritten == 0)
            {
                return FilesystemReturnCode::WRITE_FAILED;
            }
        
            return FilesystemReturnCode::FILESYSTEM_OK;
        }

        // Loads the settings file from the given path
        // static FilesystemReturnCode LoadSettingsFile(std::string filename)
        // {
        //     _SettingsFile.clear();
        //     auto returncode = ReadFile(filename, _SettingsFile);
        
        //     if (returncode == FilesystemReturnCode::FILESYSTEM_OK)
        //     {
        //         _SettingsFilename = filename;
        //     }
        
        //     return returncode;
        // }

        // Settings file getter
        static JsonDocument &SettingsFile() { return _SettingsFile; }

        static FilesystemReturnCode LoadSettingsFile(JsonDocument &doc) { return ReadFile(SettingsFileName(), doc); }

        static std::string &SettingsFileName()
        {
            static std::string _SettingsFile = "/Settings.msgpk";
            return _SettingsFile;
        }

        // print settings file to Serial
        static void PrintSettingsFile() { serializeJson(_SettingsFile, Serial); }

        // Settings file setter
        static FilesystemReturnCode WriteSettingsFile(std::string filename, JsonDocument &doc) 
        {
            auto returncode = WriteFile(SettingsFileName(), doc);
            if (returncode == FilesystemReturnCode::FILESYSTEM_OK && &doc != &_SettingsFile)
            {
                _SettingsFile.set(doc);
            }
            return returncode;
        }

        static FilesystemReturnCode WriteSettingsFileToFlash()
        {
            if (SettingsFileName().length() == 0 || SettingsFile().isNull())
            {
                Serial.println("Settings file not loaded.");
                return FilesystemReturnCode::WRITE_FAILED;
            }

            
            auto returnCode = WriteFile(SettingsFileName(), SettingsFile());
            if (returnCode == FilesystemReturnCode::FILESYSTEM_OK)
            {
                _SettingsUpdated.Invoke(SettingsFile());
            }

            return returnCode;
        }

        static void RpcGetSettingsFile(JsonDocument &doc)
        {
            doc.clear();
            LoadSettingsFile(doc);
        }

        static void RpcUpdateSettingsFile(JsonDocument &doc)
        {
            // _SettingsFile.set(doc["Settings"].as<JsonObject>());
            // WriteSettingsFileToFlash();

            ArduinoJson::serializeJson(doc["Settings"], Serial);

            doc.clear();
            doc["Success"] = true;
        }

        static void RpcUpdateSetting(JsonDocument &doc) 
        {
            auto key = doc["SettingKey"].as<std::string>();
            auto value = doc["SettingValue"].as<std::string>();

            // serializeJson(doc, Serial);

            SettingsFile()[key]["cfgVal"] = value;

            doc.clear();
            doc["Success"] = WriteSettingsFileToFlash() == FilesystemReturnCode::FILESYSTEM_OK;
        }

        static void RpcUpdateSettings(JsonDocument &doc) 
        {
            bool success = true;

            if (doc.containsKey("Settings") && doc["Settings"].is<JsonArray>())
            {
                auto settingsArray = doc["Settings"].as<JsonArray>();
                for (auto setting : settingsArray)
                {
                    auto key = setting["SettingKey"].as<std::string>();
                    auto value = setting["SettingValue"].as<std::string>();

                    SettingsFile()[key]["cfgVal"] = value;
                }

                success = WriteSettingsFileToFlash() == FilesystemReturnCode::FILESYSTEM_OK;
            }
            else
            {
                success = false;
            }

            doc.clear();
            doc["Success"] = success;
        }

        // SettingsUpdated event handler getter
        static EventHandlerT<ArduinoJson::JsonDocument &> &SettingsUpdated() { return _SettingsUpdated; }

    protected:
        // Settings File
        static DynamicJsonDocument _SettingsFile;
        static std::string _SettingsFilename;

        // Event handler for settings file updates
        static EventHandlerT<ArduinoJson::JsonDocument &> _SettingsUpdated;
    };
};