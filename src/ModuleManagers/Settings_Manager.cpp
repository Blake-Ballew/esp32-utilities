#include "Settings_Manager.h"

ArduinoJson::DynamicJsonDocument Settings_Manager::settings = ArduinoJson::DynamicJsonDocument(SIZE_SETTINGS_OBJECT);
ArduinoJson::DynamicJsonDocument Settings_Manager::savedMessages = ArduinoJson::DynamicJsonDocument(SIZE_SAVED_MESSAGES_OBJECT);
ArduinoJson::DynamicJsonDocument Settings_Manager::savedCoordinates = ArduinoJson::DynamicJsonDocument(SIZE_COORDS_OBJECT);

// EepromStream Settings_Manager::eepromStream = EepromStream(EEPROM_SETTINGS_ADDR, EEPROM_SETTINGS_SIZE);

void Settings_Manager::init()
{
#ifdef USE_SPIFFS
    SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);
#else
    EEPROM.begin(SIZE_SETTINGS_OBJECT);
#endif
    // #if UPLOAD_SETTINGS != 1
    readSettingsFromEEPROM();
    readMessagesFromEEPROM();
    readCoordsFromEEPROM();

    Serial.println("SPIFFS Info:");
    Serial.printf("Total Bytes: %u\n", SPIFFS.totalBytes());
    Serial.printf("Used Bytes: %u\n", SPIFFS.usedBytes());
    Serial.printf("Free Bytes: %u\n", SPIFFS.totalBytes() - SPIFFS.usedBytes());
    // #endif
}

bool Settings_Manager::readSettingsFromEEPROM()
{
#if DEBUG == 1
    Serial.println();
    Serial.println("Reading settings from EEPROM");
#endif
#ifdef USE_SPIFFS
    File settingsFile = SPIFFS.open("/settings.json", FILE_READ);
    if (!settingsFile)
    {
        Serial.println("Failed to open settings file");
        return false;
    }
    deserializeMsgPack(settings, settingsFile);
    settingsFile.close();
    settings.shrinkToFit();
    return true;
#else
    EepromStream eepromStream = EepromStream(EEPROM_SETTINGS_ADDR, SIZE_SETTINGS_OBJECT);
    deserializeMsgPack(settings, eepromStream);
    settings.shrinkToFit();
    return true;
#endif
}

bool Settings_Manager::writeSettingsToEEPROM()
{
#ifdef USE_SPIFFS
    File settingsFile = SPIFFS.open("/settings.json", FILE_WRITE);
    if (!settingsFile)
    {
        Serial.println("Failed to open settings file");
        return false;
    }
    serializeMsgPack(settings, settingsFile);
    settingsFile.close();
    return true;
#else
    EepromStream eepromStream = EepromStream(EEPROM_SETTINGS_ADDR, SIZE_SETTINGS_OBJECT);
    serializeMsgPack(settings, eepromStream);
    eepromStream.flush();
    return true;
#endif
}

bool Settings_Manager::readSettingsFromSerial()
{
    if (Serial.available() == 0)
    {
        return false;
    }
    while (Serial.available() > 0)
    {
        deserializeJson(settings, Serial);
    }
    settings.shrinkToFit();
    return true;
}

bool Settings_Manager::writeSettingsToSerial()
{
    if (Serial.availableForWrite() == 0)
    {
        return false;
    }
    serializeJson(settings, Serial);
    Serial.println();
    return true;
}

bool Settings_Manager::readMessagesFromEEPROM()
{
#ifdef USE_SPIFFS
    File msgFile = SPIFFS.open("/messages.json", FILE_READ);
    if (!msgFile)
    {
        Serial.println("Failed to open messages file");
        return false;
    }
    deserializeMsgPack(savedMessages, msgFile);
    msgFile.close();
    return true;
#else
    EepromStream eepromStream = EepromStream(EEPROM_SAVED_MESSAGES_ADDR, SIZE_SAVED_MESSAGES_OBJECT);
    deserializeMsgPack(savedMessages, eepromStream);
    return true;
#endif
}

bool Settings_Manager::writeMessagesToEEPROM()
{
#ifdef USE_SPIFFS
    File msgFile = SPIFFS.open("/messages.json", FILE_WRITE);
    if (!msgFile)
    {
        Serial.println("Failed to open messages file");
        return false;
    }
    serializeMsgPack(savedMessages, msgFile);
    msgFile.close();
    return true;
#else
    EepromStream eepromStream = EepromStream(EEPROM_SAVED_MESSAGES_ADDR, SIZE_SAVED_MESSAGES_OBJECT);
    serializeMsgPack(savedMessages, eepromStream);
    eepromStream.flush();
    return true;
#endif
}

bool Settings_Manager::readMessagesFromSerial()
{
    if (Serial.available() == 0)
    {
        return false;
    }
    while (Serial.available() > 0)
    {
        deserializeJson(savedMessages, Serial);
    }
    return true;
}

bool Settings_Manager::writeMessagesToSerial()
{
    if (Serial.availableForWrite() == 0)
    {
        return false;
    }
    serializeJson(savedMessages, Serial);
    Serial.println();
    return true;
}

bool Settings_Manager::addMessage(const char *msg, size_t msgLength)
{
    if (msgLength > maxMsgLength)
    {
#if DEBUG == 1
        Serial.println("Message too long");
#endif
        return false;
    }
    if (savedMessages["Messages"].as<JsonArray>().size() >= maxMsges)
    {
#if DEBUG == 1
        Serial.println("Too many messages");
#endif
        return false;
    }
    savedMessages["Messages"].as<JsonArray>().add(msg);
    return true;
}

bool Settings_Manager::deleteMessage(size_t msgIdx)
{

    if (msgIdx >= savedMessages["Messages"].size())
    {
#if DEBUG == 1
        Serial.printf("msgIdx: %d is out of bounds\n", msgIdx);
#endif
        return false;
    }
#if DEBUG == 1
    Serial.print("Deleting message at index: ");
    Serial.println(msgIdx);
    Serial.printf("Message: %s\n", savedMessages["Messages"][msgIdx].as<const char *>());
    Serial.printf("Array size before: %d\n", savedMessages["Messages"].as<JsonArray>().size());
#endif
    savedMessages["Messages"].as<JsonArray>().remove(msgIdx);
#if DEBUG == 1
    Serial.printf("Array size after: %d\n", savedMessages["Messages"].as<JsonArray>().size());
#endif
    return true;
}

bool Settings_Manager::deleteMessage(JsonArray::iterator msgIt)
{
    savedMessages["Messages"].as<JsonArray>().remove(msgIt);
    return true;
}

size_t Settings_Manager::getNumMsges()
{
    return savedMessages["Messages"].as<JsonArray>().size();
}

JsonArray::iterator Settings_Manager::getMsgIteratorBegin()
{
    return savedMessages["Messages"].as<JsonArray>().begin();
}

JsonArray::iterator Settings_Manager::getMsgIteratorEnd()
{
    return savedMessages["Messages"].as<JsonArray>().end();
}

void Settings_Manager::WriteMessagesToJSON(ArduinoJson::JsonDocument &doc)
{
    // Create nested array "messages" if it does not exist
    if (!doc.containsKey("messages"))
    {
        doc.createNestedArray("messages");
    }

    // iterate through saved messages and add them to the JSON document
    for (auto msg : savedMessages["Messages"].as<JsonArray>())
    {
        doc["messages"].add(msg.as<std::string>());
    }
}

bool Settings_Manager::readCoordsFromEEPROM()
{
    File coordFile = SPIFFS.open("/coords.json", FILE_READ);
    if (!coordFile)
    {
        Serial.println("Failed to open coords file");
        return false;
    }
    deserializeMsgPack(savedCoordinates, coordFile);
    coordFile.close();
    return true;
}

bool Settings_Manager::writeCoordsToEEPROM()
{
    File coordFile = SPIFFS.open("/coords.json", FILE_WRITE);
    if (!coordFile)
    {
        Serial.println("Failed to open coords file");
        return false;
    }
    serializeMsgPack(savedCoordinates, coordFile);
    coordFile.close();
    return true;
}

bool Settings_Manager::readCoordsFromSerial()
{
    if (Serial.available() == 0)
    {
        return false;
    }
    while (Serial.available() > 0)
    {
        deserializeJson(savedCoordinates, Serial);
    }
    return true;
}

bool Settings_Manager::writeCoordsToSerial()
{
    if (Serial.availableForWrite() == 0)
    {
        return false;
    }
    serializeJson(savedCoordinates, Serial);
    Serial.println();
    return true;
}

bool Settings_Manager::addCoordinate(const char *name, double lat, double lon)
{
    if (savedCoordinates["Coords"].as<JsonArray>().size() >= maxCoords)
    {
#if DEBUG == 1
        Serial.println("Too many coordinates");
#endif
        return false;
    }
    JsonObject coord = savedCoordinates["Coords"].as<JsonArray>().createNestedObject();
    coord["n"] = name;
    coord["la"] = lat;
    coord["lo"] = lon;

    writeCoordsToEEPROM();
    return true;
}

bool Settings_Manager::deleteCoordinate(size_t coordIdx)
{
    if (coordIdx >= savedCoordinates["Coords"].size())
    {
#if DEBUG == 1
        Serial.printf("coordIdx: %d is out of bounds\n", coordIdx);
#endif
        return false;
    }
#if DEBUG == 1
    Serial.print("Deleting coordinate at index: ");
    Serial.println(coordIdx);
    Serial.printf("Coordinate: %s\n", savedCoordinates["Coords"][coordIdx]["n"].as<const char *>());
    Serial.printf("Array size before: %d\n", savedCoordinates["Coords"].as<JsonArray>().size());
#endif
    savedCoordinates["Coords"].as<JsonArray>().remove(coordIdx);
#if DEBUG == 1
    Serial.printf("Array size after: %d\n", savedCoordinates["Coords"].as<JsonArray>().size());
#endif
    return true;
}

size_t Settings_Manager::getNumCoords()
{
    return savedCoordinates["Coords"].as<JsonArray>().size();
}

JsonArray::iterator Settings_Manager::getCoordIteratorBegin()
{
    return savedCoordinates["Coords"].as<JsonArray>().begin();
}

JsonArray::iterator Settings_Manager::getCoordIteratorEnd()
{
    return savedCoordinates["Coords"].as<JsonArray>().end();
}

void Settings_Manager::WriteCoordinatesToJSON(ArduinoJson::JsonDocument &doc)
{
    // Create nested array "locations" if it does not exist
    if (!doc.containsKey("locations"))
    {
        doc.createNestedArray("locations");
    }

    // iterate through saved coordinates and add them to the JSON document
    for (auto coord : savedCoordinates["Coords"].as<JsonArray>())
    {
        JsonObject location = doc["locations"].createNestedObject();
        location["n"] = coord["n"].as<std::string>();
        location["la"] = coord["la"].as<double>();
        location["lo"] = coord["lo"].as<double>();
    }
}

bool Settings_Manager::readStatusesFromEEPROM(ArduinoJson::JsonDocument &doc)
{
    EepromStream statusStream = EepromStream(EEPROM_STATUSES_ADDR, SIZE_STATUSES_OBJECT);
    deserializeMsgPack(doc, statusStream);
    return true;
}

bool Settings_Manager::writeStatusesToEEPROM(ArduinoJson::JsonDocument &doc)
{
    EepromStream statusStream = EepromStream(EEPROM_STATUSES_ADDR, SIZE_STATUSES_OBJECT);
    serializeMsgPack(doc, statusStream);
    statusStream.flush();
    return true;
}

void Settings_Manager::flashSettings()
{

    DynamicJsonDocument doc(SIZE_SETTINGS_OBJECT);
    EepromStream eepromStream = EepromStream(EEPROM_SETTINGS_ADDR, SIZE_SETTINGS_OBJECT);
    JsonObject User = doc.createNestedObject("User");

    User["UserID"] = esp_random();

    JsonObject User_Name = User.createNestedObject("Name");
    User_Name["cfgType"] = 10;
    User_Name["cfgVal"] = "User";
    User_Name["dftVal"] = "User";
    User_Name["maxLen"] = 12;
    doc["Device"]["24HTime"] = false;

    JsonObject User_Theme_Red = User.createNestedObject("Theme Red");
    User_Theme_Red["cfgType"] = 8;
    User_Theme_Red["cfgVal"] = 0;
    User_Theme_Red["dftVal"] = 0;
    User_Theme_Red["maxVal"] = 255;
    User_Theme_Red["minVal"] = 0;
    User_Theme_Red["incVal"] = 5;
    User_Theme_Red["signed"] = false;

    JsonObject User_Theme_Green = User.createNestedObject("Theme Green");
    User_Theme_Green["cfgType"] = 8;
    User_Theme_Green["cfgVal"] = 255;
    User_Theme_Green["dftVal"] = 255;
    User_Theme_Green["maxVal"] = 255;
    User_Theme_Green["minVal"] = 0;
    User_Theme_Green["incVal"] = 5;
    User_Theme_Green["signed"] = false;

    JsonObject User_Theme_Blue = User.createNestedObject("Theme Blue");
    User_Theme_Blue["cfgType"] = 8;
    User_Theme_Blue["cfgVal"] = 0;
    User_Theme_Blue["dftVal"] = 0;
    User_Theme_Blue["maxVal"] = 255;
    User_Theme_Blue["minVal"] = 0;
    User_Theme_Blue["incVal"] = 5;
    User_Theme_Blue["signed"] = false;
    doc["Device"]["24HTime"] = false;

    JsonObject Radio = doc.createNestedObject("Radio");

    JsonObject Radio_Frequency = Radio.createNestedObject("Frequency");
    Radio_Frequency["cfgType"] = 9;
    Radio_Frequency["cfgVal"] = 914.9;
    Radio_Frequency["dftVal"] = 914.9;
    Radio_Frequency["maxVal"] = 914.9;
    Radio_Frequency["minVal"] = 902.3;
    Radio_Frequency["incVal"] = 0.2;

    JsonObject Radio_NodeID = Radio.createNestedObject("NodeID");
    Radio_NodeID["cfgType"] = 8;
    Radio_NodeID["cfgVal"] = (ESP.getEfuseMac() & 0xFF) == 255 ? 254 : (ESP.getEfuseMac() & 0xFF);
    Radio_NodeID["dftVal"] = (ESP.getEfuseMac() & 0xFF) == 255 ? 254 : (ESP.getEfuseMac() & 0xFF);
    Radio_NodeID["maxVal"] = 254;
    Radio_NodeID["minVal"] = 0;
    Radio_NodeID["incVal"] = 1;
    Radio_NodeID["signed"] = false;

    JsonObject Radio_Modem_Config = Radio.createNestedObject("Modem Config");
    Radio_Modem_Config["cfgType"] = 11;
    Radio_Modem_Config["cfgVal"] = 4;
    Radio_Modem_Config["dftVal"] = 4;

    JsonArray Radio_Modem_Config_vals = Radio_Modem_Config.createNestedArray("vals");
    Radio_Modem_Config_vals.add(0);
    Radio_Modem_Config_vals.add(1);
    Radio_Modem_Config_vals.add(2);
    Radio_Modem_Config_vals.add(3);
    Radio_Modem_Config_vals.add(4);

    JsonArray Radio_Modem_Config_valTxt = Radio_Modem_Config.createNestedArray("valTxt");
    Radio_Modem_Config_valTxt.add("125 kHz, 4/5, 128");
    Radio_Modem_Config_valTxt.add("500 kHz, 4/5, 128");
    Radio_Modem_Config_valTxt.add("31.25 kHz, 4/8, 512");
    Radio_Modem_Config_valTxt.add("125 kHz, 4/8, 4096");
    Radio_Modem_Config_valTxt.add("125 khz, 4/5, 2048");
    Radio["Router"] = false;

    JsonObject Radio_Broadcast_Retries = Radio.createNestedObject("Broadcast Retries");
    Radio_Broadcast_Retries["cfgType"] = 8;
    Radio_Broadcast_Retries["cfgVal"] = 3;
    Radio_Broadcast_Retries["dftVal"] = 3;
    Radio_Broadcast_Retries["maxVal"] = 5;
    Radio_Broadcast_Retries["minVal"] = 1;
    Radio_Broadcast_Retries["incVal"] = 1;
    Radio_Broadcast_Retries["signed"] = false;
    Radio["Router"] = false;

    JsonObject Compass = doc.createNestedObject("Compass");
    Compass["useCalibration"] = false;
    Compass["X_MIN"] = 0;
    Compass["X_MAX"] = 0;
    Compass["Y_MIN"] = 0;
    Compass["Y_MAX"] = 0;
    Compass["Z_MIN"] = 0;
    Compass["Z_MAX"] = 0;
    doc["GPS"]["refreshDelayms"] = 5000;

    // Write default messages to EEPROM
    DynamicJsonDocument msgDoc(SIZE_SAVED_MESSAGES_OBJECT);
    JsonArray msgArray = msgDoc.createNestedArray("Messages");
    msgArray.add("Meet here");
    msgArray.add("Follow me");
    msgArray.add("Point of interest");

    DynamicJsonDocument coordDoc(SIZE_COORDS_OBJECT);

    JsonArray Coords = coordDoc["Coords"].to<JsonArray>();

    JsonObject Coords_0 = Coords.createNestedObject();
    Coords_0["la"] = 33.7488;
    Coords_0["lo"] = -84.3877;
    Coords_0["n"] = "Atlanta";

    JsonObject Coords_1 = Coords.createNestedObject();
    Coords_1["la"] = 37.7749;
    Coords_1["lo"] = -122.4194;
    Coords_1["n"] = "San Francisco";

    JsonObject Coords_2 = Coords.createNestedObject();
    Coords_2["la"] = 40.7128;
    Coords_2["lo"] = -74.0060;
    Coords_2["n"] = "New York";

#ifdef USE_SPIFFS
    // Save settings
    File settingsFile = SPIFFS.open("/settings.json", FILE_WRITE);
    if (!settingsFile)
    {
        Serial.println("Failed to open settings file");
        return;
    }
    serializeJson(doc, Serial);
    serializeMsgPack(doc, settingsFile);

    // Save messages
    File msgFile = SPIFFS.open("/messages.json", FILE_WRITE);
    if (!msgFile)
    {
        Serial.println("Failed to open messages file");
        return;
    }
    serializeJson(msgDoc, Serial);
    serializeMsgPack(msgDoc, msgFile);

    // Save coords
    File coordFile = SPIFFS.open("/coords.json", FILE_WRITE);
    if (!coordFile)
    {
        Serial.println("Failed to open coords file");
        return;
    }
    serializeJson(coordDoc, Serial);
    serializeMsgPack(coordDoc, coordFile);

    coordFile.close();
    settingsFile.close();
    msgFile.close();
    msgDoc.clear();
    doc.clear();
    coordDoc.clear();
#else
    EepromStream msgStream = EepromStream(EEPROM_SAVED_MESSAGES_ADDR, SIZE_SAVED_MESSAGES_OBJECT);
    serializeJson(msgDoc, Serial);
    serializeMsgPack(msgDoc, msgStream);
    serializeJson(doc, Serial);
    serializeMsgPack(doc, eepromStream);
    msgStream.flush();
    eepromStream.flush();
    msgDoc.clear();
    doc.clear();
#endif
}

JsonVariantType Settings_Manager::getVariantType(ArduinoJson::JsonVariant variant)
{
    if (variant.isNull())
    {
        return JSON_VARIANT_TYPE_NULL;
    }
    else if (variant.is<bool>())
    {
        return JSON_VARIANT_TYPE_BOOLEAN;
    }
    else if (variant.is<int>())
    {
        return JSON_VARIANT_TYPE_INTEGER;
    }
    else if (variant.is<float>())
    {
        return JSON_VARIANT_TYPE_FLOAT;
    }
    else if (variant.is<const char *>())
    {
        return JSON_VARIANT_TYPE_STRING;
    }
    else if (variant.is<ArduinoJson::JsonArray>())
    {
        return JSON_VARIANT_TYPE_ARRAY;
    }
    else if (variant.is<ArduinoJson::JsonObject>())
    {
        ArduinoJson::JsonObject obj = variant.as<ArduinoJson::JsonObject>();
        if (obj.containsKey("cfgType") &&
            obj.containsKey("cfgVal") &&
            obj.containsKey("dftVal"))
        {
            switch (obj["cfgType"].as<uint8_t>())
            {
            case (uint8_t)JSON_VARIANT_CONFIGURABLE_STRING:
                if (obj.containsKey("maxLen"))
                    return JSON_VARIANT_CONFIGURABLE_STRING;
            case (uint8_t)JSON_VARIANT_CONFIGURABLE_INTEGER:
                if (obj.containsKey("maxVal") &&
                    obj.containsKey("minVal") &&
                    obj.containsKey("incVal") &&
                    obj.containsKey("signed"))
                    return JSON_VARIANT_CONFIGURABLE_INTEGER;
            case (uint8_t)JSON_VARIANT_CONFIGURABLE_FLOAT:
                if (obj.containsKey("minVal") &&
                    obj.containsKey("maxVal") &&
                    obj.containsKey("incVal"))
                    return JSON_VARIANT_CONFIGURABLE_FLOAT;
            case (uint8_t)JSON_VARIANT_CONFIGURABLE_BOOL:
                return JSON_VARIANT_CONFIGURABLE_BOOL;
            case (uint8_t)JSON_VARIANT_CONFIGURABLE_ENUM:
                if (obj.containsKey("vals") &&
                    obj.containsKey("valTxt"))
                    return JSON_VARIANT_CONFIGURABLE_ENUM;
            default:
                return JSON_VARIANT_TYPE_OBJECT;
            }
        }
        return JSON_VARIANT_TYPE_OBJECT;
    }
    else
    {
        return JSON_VARIANT_TYPE_NULL;
    }
}
