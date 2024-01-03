#pragma once

#include "globalDefines.h"
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <StreamUtils.h>
// #include <FS.h>
#include <SPIFFS.h>

class Settings_Manager
{
public:
    static ArduinoJson::DynamicJsonDocument settings;
    static ArduinoJson::DynamicJsonDocument savedMessages;
    // static EepromStream eepromStream;

    static void init();

    // Saved Messages
    static bool readMessagesFromEEPROM();
    static bool writeMessagesToEEPROM();
    static bool readMessagesFromSerial();
    static bool writeMessagesToSerial();
    static bool addMessage(const char *msg, size_t msgLength);
    static bool deleteMessage(size_t msgIdx);
    static bool deleteMessage(JsonArray::iterator msgIt);
    static size_t getNumMsges();
    static JsonArray::iterator getMsgIteratorBegin();
    static JsonArray::iterator getMsgIteratorEnd();

    static bool readSettingsFromEEPROM();
    static bool writeSettingsToEEPROM();

    static bool readSettingsFromSerial();
    static bool writeSettingsToSerial();

    static bool readStatusesFromEEPROM(ArduinoJson::JsonDocument &doc);
    static bool writeStatusesToEEPROM(ArduinoJson::JsonDocument &doc);

    static void flashSettings();

    const static int maxMsgLength = 23;
    const static int maxMsges = 100;
};
