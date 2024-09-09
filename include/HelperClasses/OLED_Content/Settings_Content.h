#pragma once

#include "FilesystemUtils.h"
#include "Settings_Manager.h"
#include "OLED_Content.h"
#include <ArduinoJson.h>
#include <stack>

/* enum JSON_VARIANT_TYPE
{
    JSON_VARIANT_TYPE_NULL = 0,
    JSON_VARIANT_TYPE_BOOLEAN,
    JSON_VARIANT_TYPE_INTEGER,
    JSON_VARIANT_TYPE_FLOAT,
    JSON_VARIANT_TYPE_STRING,
    JSON_VARIANT_TYPE_ARRAY,
    JSON_VARIANT_TYPE_OBJECT,
    JSON_VARIANT_CONFIGURABLE_BOOL,
    JSON_VARIANT_CONFIGURABLE_INTEGER,
    JSON_VARIANT_CONFIGURABLE_FLOAT,
    JSON_VARIANT_CONFIGURABLE_STRING,
    JSON_VARIANT_CONFIGURABLE_ENUM,
}; */

/* struct JsonVariantStack
{
    ArduinoJson::JsonVariant variant;
    JSON_VARIANT_TYPE type;
    size_t idx;
    JsonVariantStack *next;
}; */

class Settings_Content : public OLED_Content
{
public:
    // JsonVariantStack *settings;
    // uint16_t settingsIndex;

    Settings_Content();
    Settings_Content(JsonDocument &settings);
    ~Settings_Content();
    void printContent();
    void encUp();
    void encDown();
    void popVariant(bool printAfter = false);
    void pushVariant(bool printAfter = false);
    void refresh();
    size_t getVariantDepth();
    void printVariantValue(ArduinoJson::JsonVariant variant);
    JsonVariantType getVariantType();
    JsonVariantType getSelectionVariantType();
    JsonVariant getCurrentVariant();
    JsonVariant getSelectionVariant();

    void saveReturnValueFromEditState(JsonVariant returnData);
    DynamicJsonDocument *getEditStateInput();

protected:
    std::stack<JsonVariantStackNode> variantStack;
    JsonVariantStackNode currentNode;
};