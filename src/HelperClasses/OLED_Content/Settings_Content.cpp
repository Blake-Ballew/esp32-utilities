#include "Settings_Content.h"

Settings_Content::Settings_Content(JsonDocument &settings)
{
    this->type = ContentType::SETTINGS;
    ArduinoJson::JsonVariant variant = settings.as<ArduinoJson::JsonVariant>();
    currentNode.variant = variant;
    currentNode.type = Settings_Manager::getVariantType(variant);
    currentNode.idx = 0;
}

Settings_Content::Settings_Content()
{
    Settings_Content(Settings_Manager::settings);
}

Settings_Content::~Settings_Content()
{
}

void Settings_Content::printContent()
{
#if DEBUG == 1
    Serial.println("Settings_Content::printContent()");
#endif
    auto variantType = Settings_Manager::getVariantType(currentNode.variant);
    this->display->fillRect(6, 8, OLED_WIDTH - 12, OLED_HEIGHT - 16, SSD1306_BLACK);

    switch (variantType)
    {
    case (uint8_t)JSON_VARIANT_TYPE_NULL:
        break;
    case (uint8_t)JSON_VARIANT_TYPE_OBJECT:
    {
        ArduinoJson::JsonObject::iterator it = currentNode.variant.as<ArduinoJson::JsonObject>().begin();
        it += currentNode.idx;
        size_t textLength = it->key().size();
        this->display->setCursor(Display_Utils::centerTextHorizontal(it->key().c_str()), 8);
        this->display->print(it->key().c_str());
        printVariantValue(it->value());
        break;
    }
    case (uint8_t)JSON_VARIANT_TYPE_ARRAY:
    {
        ArduinoJson::JsonArray::iterator it = currentNode.variant.as<ArduinoJson::JsonArray>().begin();
        it += currentNode.idx;
        printVariantValue(*it);
        break;
    }
    case (uint8_t)JSON_VARIANT_TYPE_BOOLEAN:
    {
        this->display->setCursor((OLED_WIDTH / 2) - 24, OLED_HEIGHT / 2 - 4);
        if (currentNode.variant.as<bool>())
        {
            this->display->print("true");
        }
        else
        {
            this->display->print("false");
        }
        break;
    }
    case (uint8_t)JSON_VARIANT_TYPE_INTEGER:
    {
        this->display->setCursor((OLED_WIDTH / 2) - 24, OLED_HEIGHT / 2 - 4);
        this->display->print(currentNode.variant.as<int64_t>());
        break;
    }
    case (uint8_t)JSON_VARIANT_TYPE_FLOAT:
    {
        this->display->setCursor((OLED_WIDTH / 2) - 24, OLED_HEIGHT / 2 - 4);
        this->display->print(currentNode.variant.as<float>());
        break;
    }
    case (uint8_t)JSON_VARIANT_TYPE_STRING:
    {
        this->display->setCursor((OLED_WIDTH / 2) - 24, OLED_HEIGHT / 2 - 4);
        this->display->print(currentNode.variant.as<const char *>());
        break;
    }
    default:
        break;
    }
    display->display();
}

void Settings_Content::encUp()
{
    auto variantType = Settings_Manager::getVariantType(currentNode.variant);

    switch (variantType)
    {
    case JSON_VARIANT_TYPE_OBJECT:
        if (currentNode.idx == 0)
        {
            this->currentNode.idx = currentNode.variant.as<ArduinoJson::JsonObject>().size() - 1;
        }
        else
        {
            this->currentNode.idx--;
        }
        this->printContent();
        break;
    case JSON_VARIANT_TYPE_ARRAY:
        if (currentNode.idx == 0)
        {
            this->currentNode.idx = currentNode.variant.as<ArduinoJson::JsonArray>().size() - 1;
        }
        else
        {
            this->currentNode.idx--;
        }
        this->printContent();
        break;
    default:
        break;
    }
}

void Settings_Content::encDown()
{
    auto variantType = Settings_Manager::getVariantType(currentNode.variant);

    switch (variantType)
    {
    case JSON_VARIANT_TYPE_OBJECT:
        if (currentNode.idx == currentNode.variant.as<ArduinoJson::JsonObject>().size() - 1)
        {
            currentNode.idx = 0;
        }
        else
        {
            currentNode.idx++;
        }
        this->printContent();
        break;
    case JSON_VARIANT_TYPE_ARRAY:
        if (currentNode.idx == currentNode.variant.as<ArduinoJson::JsonArray>().size() - 1)
        {
            currentNode.idx = 0;
        }
        else
        {
            currentNode.idx++;
        }
        this->printContent();
        break;
    default:
        break;
    }
}

void Settings_Content::refresh()
{
    size_t idx = currentNode.idx;
    this->popVariant(false);
    this->pushVariant(false);
    this->currentNode.idx = idx;
    this->printContent();
}

size_t Settings_Content::getVariantDepth()
{
#if DEBUG == 1
    Serial.printf("=============Settings_Content::getVariantDepth(): %d=============\n", variantStack.size());
#endif
    return variantStack.size();
}

void Settings_Content::popVariant(bool printAfter)
{
    if (variantStack.size() > 0)
    {
        JsonVariantStackNode node = variantStack.top();
        variantStack.pop();
        currentNode = node;
    }
    if (printAfter)
    {
        this->printContent();
    }
}

void Settings_Content::pushVariant(bool printAfter)
{
    auto variantType = Settings_Manager::getVariantType(currentNode.variant);
    if (variantType == JSON_VARIANT_TYPE_OBJECT)
    {
        ArduinoJson::JsonObject::iterator it = currentNode.variant.as<ArduinoJson::JsonObject>().begin();
        it += currentNode.idx;
        JsonVariantStackNode node;
        node.variant = it->value();
        node.type = Settings_Manager::getVariantType(it->value());
        node.idx = 0;
        variantStack.push(currentNode);
        currentNode = node;
    }
    else if (variantType == JSON_VARIANT_TYPE_ARRAY)
    {
        ArduinoJson::JsonArray::iterator it = currentNode.variant.as<ArduinoJson::JsonArray>().begin();
        it += currentNode.idx;
        JsonVariantStackNode node;
        node.variant = *it;
        node.type = Settings_Manager::getVariantType(*it);
        node.idx = 0;
        variantStack.push(currentNode);
        currentNode = node;
    }

    if (printAfter)
    {
        this->printContent();
    }
}

void Settings_Content::printVariantValue(ArduinoJson::JsonVariant variant)
{
    auto variantType = Settings_Manager::getVariantType(variant);
    switch (variantType)
    {
    case JSON_VARIANT_TYPE_ARRAY:
        display->setCursor(Display_Utils::centerTextHorizontal(9), 16);
        display->print("[ . . . ]");
        break;
    case JSON_VARIANT_TYPE_OBJECT:
        display->setCursor(Display_Utils::centerTextHorizontal(9), 16);
        display->print("{ . . . }");
        break;
    case JSON_VARIANT_TYPE_BOOLEAN:

        if (variant.as<bool>())
        {
            display->setCursor(Display_Utils::centerTextHorizontal(4), 16);
            display->print("true");
        }
        else
        {
            display->setCursor(Display_Utils::centerTextHorizontal(5), 16);
            display->print("false");
        }
        break;
    case JSON_VARIANT_TYPE_INTEGER:
    {
        int num = variant.as<int>();
        display->setCursor(Display_Utils::centerTextHorizontal(Display_Utils::getIntLength(num)), 16);
        display->print(variant.as<int>());
        break;
    }
    case JSON_VARIANT_TYPE_FLOAT:
    {
        double num = variant.as<double>();
        display->setCursor(Display_Utils::centerTextHorizontal(5), 16);
        display->print(num, 6);
        break;
    }
    case JSON_VARIANT_TYPE_STRING:
    {
        display->setCursor(Display_Utils::centerTextHorizontal(variant.as<const char *>()), 16);
        display->print(variant.as<const char *>());
        break;
    }
    case JSON_VARIANT_CONFIGURABLE_ENUM:
    {
        const char *str = variant["valTxt"][variant["cfgVal"].as<uint8_t>()].as<const char *>();
        display->setCursor(Display_Utils::centerTextHorizontal(str), 16);
        display->print(str);
        break;
    }
    case JSON_VARIANT_CONFIGURABLE_STRING:
    {
        display->setCursor(Display_Utils::centerTextHorizontal(variant["cfgVal"].as<const char *>()), 16);
        display->print(variant["cfgVal"].as<const char *>());
        break;
    }
    case JSON_VARIANT_CONFIGURABLE_INTEGER:
    {
        if (variant["signed"].as<bool>())
        {
            int32_t num = variant["cfgVal"].as<int32_t>();
            display->setCursor(Display_Utils::centerTextHorizontal(Display_Utils::getIntLength(num)), 16);
            display->print(num);
        }
        else
        {
            uint32_t num = variant["cfgVal"].as<uint32_t>();
            display->setCursor(Display_Utils::centerTextHorizontal(Display_Utils::getIntLength(num)), 16);
            display->print(num);
        }
        break;
    }
    case JSON_VARIANT_CONFIGURABLE_FLOAT:
    {
        double num = variant["cfgVal"].as<double>();
        display->setCursor(Display_Utils::centerTextHorizontal(5), 16);
        display->print(num, 6);
        break;
    }
    }
}

JsonVariantType Settings_Content::getVariantType()
{
    return currentNode.type;
}

JsonVariantType Settings_Content::getSelectionVariantType()
{
    auto variantType = currentNode.type;
    switch (variantType)
    {
    case JsonVariantType::JSON_VARIANT_TYPE_OBJECT:
    {
        ArduinoJson::JsonObject::iterator it = currentNode.variant.as<ArduinoJson::JsonObject>().begin();
        it += currentNode.idx;
        return Settings_Manager::getVariantType(it->value());
    }
    case JsonVariantType::JSON_VARIANT_TYPE_ARRAY:
    {
        ArduinoJson::JsonArray::iterator it = currentNode.variant.as<ArduinoJson::JsonArray>().begin();
        it += currentNode.idx;
        return Settings_Manager::getVariantType(*it);
    }
    default:
        return JsonVariantType::JSON_VARIANT_TYPE_NULL;
    }
}

JsonVariant Settings_Content::getCurrentVariant()
{
    return currentNode.variant;
}

void Settings_Content::saveReturnValueFromEditState(JsonVariant returnData)
{
    if (currentNode.type == JSON_VARIANT_CONFIGURABLE_BOOL ||
        currentNode.type == JSON_VARIANT_CONFIGURABLE_INTEGER ||
        currentNode.type == JSON_VARIANT_CONFIGURABLE_FLOAT ||
        currentNode.type == JSON_VARIANT_CONFIGURABLE_STRING ||
        currentNode.type == JSON_VARIANT_CONFIGURABLE_ENUM)
    {
        if (!returnData.is<const char *>()) 
        {
            currentNode.variant["cfgVal"] = returnData;
        }
        else
        {
            const char *str = returnData.as<const char *>();
            #if DEBUG == 1
            Serial.print("Settings_Content::saveReturnValueFromEditState(): str: ");
            Serial.println(str);
            #endif
            char *newStr = new char[strlen(str) + 1];
            strcpy(newStr, str);
            newStr[strlen(str)] = '\0';
            currentNode.variant["cfgVal"] = (const char *)newStr;
        }
    }
}

DynamicJsonDocument *Settings_Content::getEditStateInput()
{
    #if DEBUG == 1
    Serial.print("Settings_Content::getEditStateInput(): currentNode.type: ");
    Serial.println(currentNode.type);
    #endif
    switch (currentNode.type)
    {
    case JSON_VARIANT_CONFIGURABLE_BOOL:
    {
        DynamicJsonDocument *doc = new DynamicJsonDocument(32);
        (*doc)["cfgVal"] = currentNode.variant["cfgVal"].as<bool>();
        return doc;
    }
    case JSON_VARIANT_CONFIGURABLE_INTEGER:
    {
        DynamicJsonDocument *doc = new DynamicJsonDocument(128);
        (*doc)["cfgVal"] = currentNode.variant["cfgVal"].as<int>();
        (*doc)["minVal"] = currentNode.variant["minVal"].as<int>();
        (*doc)["maxVal"] = currentNode.variant["maxVal"].as<int>();
        (*doc)["incVal"] = currentNode.variant["incVal"].as<int>();
        (*doc)["signed"] = currentNode.variant["signed"].as<bool>();
        return doc;
    }
    case JSON_VARIANT_CONFIGURABLE_FLOAT:
    {
        DynamicJsonDocument *doc = new DynamicJsonDocument(64);
        (*doc)["cfgVal"] = currentNode.variant["cfgVal"].as<float>();
        (*doc)["minVal"] = currentNode.variant["minVal"].as<float>();
        (*doc)["maxVal"] = currentNode.variant["maxVal"].as<float>();
        (*doc)["incVal"] = currentNode.variant["incVal"].as<float>();
        return doc;
    }
    case JSON_VARIANT_CONFIGURABLE_STRING:
    {
        DynamicJsonDocument *doc = new DynamicJsonDocument(64);
        (*doc)["cfgVal"] = currentNode.variant["cfgVal"].as<const char *>();
        (*doc)["maxLen"] = currentNode.variant["maxLen"].as<uint8_t>();
        return doc;
    }
    case JSON_VARIANT_CONFIGURABLE_ENUM:
    {
        DynamicJsonDocument *doc = new DynamicJsonDocument(512);
        (*doc)["cfgVal"] = currentNode.variant["cfgVal"].as<uint8_t>();
        (*doc)["vals"] = currentNode.variant["vals"].as<ArduinoJson::JsonArray>();
        (*doc)["valTxt"] = currentNode.variant["valTxt"].as<ArduinoJson::JsonArray>();
        return doc;
    }
    default:
        return nullptr;
    }
}

/* JSON_VARIANT_TYPE Settings_Content::getVariantType(ArduinoJson::JsonVariant variant)
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
} */