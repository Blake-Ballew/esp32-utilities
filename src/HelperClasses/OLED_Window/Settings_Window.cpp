#include "Settings_Window.h"

Settings_Window::Settings_Window(OLED_Window *parent) : OLED_Window(parent)
{
    settingsState = new Settings_State();
    editBoolState = new Edit_Bool_State();
    editIntState = new Edit_Int_State();
    editStringState = new Edit_String_State();
    editFloatState = new Edit_Float_State();
    editEnumState = new Edit_Enum_State();

    stateList.push_back(settingsState);
    stateList.push_back(editBoolState);
    stateList.push_back(editIntState);
    stateList.push_back(editStringState);
    stateList.push_back(editFloatState);
    stateList.push_back(editEnumState);

    contentList.push_back(settingsState->renderContent);
    contentList.push_back(editBoolState->renderContent);
    contentList.push_back(editIntState->renderContent);
    contentList.push_back(editStringState->renderContent);
    contentList.push_back(editFloatState->renderContent);
    contentList.push_back(editEnumState->renderContent);

    if (Settings_Manager::settings.isNull())
    {
        Settings_Manager::flashSettings();
    }

    this->saveSettings = false;

    setInitialState(settingsState);
}

Settings_Window::~Settings_Window()
{
}

void Settings_Window::callFunctionState(uint8_t inputID)
{
    if (currentState == settingsState)
    {
        bool initializeEditSuccess = false;
        JsonVariantType editType = JSON_VARIANT_TYPE_NULL;
        auto valueToEdit = settingsState->initializeEdit(initializeEditSuccess, editType);

        if (initializeEditSuccess)
        {
            State_Transfer_Data transferData;
            transferData.callbackID = ACTION_CALL_FUNCTIONAL_WINDOW_STATE;
            transferData.inputID = inputID;
            transferData.oldState = currentState;
            transferData.serializedData = valueToEdit;

            switch (editType)
            {
            case JSON_VARIANT_CONFIGURABLE_BOOL:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editBoolState");
#endif
                transferData.newState = editBoolState;
                break;
            case JSON_VARIANT_CONFIGURABLE_INTEGER:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editIntState");
#endif
                transferData.newState = editIntState;
                break;
            case JSON_VARIANT_CONFIGURABLE_STRING:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editStringState");
#endif
                transferData.newState = editStringState;
                break;
            case JSON_VARIANT_CONFIGURABLE_FLOAT:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editFloatState");
#endif
                transferData.newState = editFloatState;
                break;
            case JSON_VARIANT_CONFIGURABLE_ENUM:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editEnumState");
#endif
                transferData.newState = editEnumState;
                break;
            default:
                break;
            }

            transferState(transferData);
        }
    }
}

void Settings_Window::returnFromFunctionState(uint8_t inputID)
{
    if (currentState == editBoolState ||
        currentState == editIntState ||
        currentState == editStringState ||
        currentState == editFloatState ||
        currentState == editEnumState)
    {
        State_Transfer_Data transferData;
        transferData.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        transferData.inputID = inputID;
        transferData.oldState = currentState;
        transferData.newState = settingsState;
        transferData.serializedData = nullptr;

        transferState(transferData);
    }
}

void Settings_Window::transferState(State_Transfer_Data &transferData)
{
#if DEBUG == 1
    Serial.println("Settings_Window::transferState()");
#endif

    transferData.oldState->exitState(transferData);

    // Child classes will process transfer data coming out of the old state if needed

    transferData.newState->enterState(transferData);

    // processStateChangeReturnCode(transferData.returnCode);

    currentState = transferData.newState;

    // Clean up
    if (transferData.serializedData != nullptr)
    {
        delete transferData.serializedData;
    }
}

// void Settings_Window::execBtnCallback(uint8_t inputID)
// {
//     uint8_t callbackID;

// #if DEBUG == 1
//     Serial.print("Settings_Window::execBtnCallback(uint8_t buttonNumber, void *arg): Button number: ");
//     Serial.println(inputID);
// #endif

//     switch (inputID)
//     {
//     case BUTTON_1:
//         callbackID = btn1CallbackID;
//         // Backspace for edit string
//         if (content->type == ContentType::EDIT_STRING)
//         {
//             editStringContent->passButtonPress(BUTTON_1);
//         }
//         break;
//     case BUTTON_2:
//         callbackID = btn2CallbackID;

//         // Save button for all edit screens
//         if (content->type == ContentType::EDIT_INT)
//         {
//             if (editIntContent->getIsSigned())
//             {
//                 int32_t value = editIntContent->getSignedInt();
// #if DEBUG == 1
//                 Serial.print("Saving signed int: ");
//                 Serial.println(value);
// #endif
//                 settingsContent->settings->variant["cfgVal"] = value;
// #if DEBUG == 1
//                 Serial.print("Signed int saved: ");
//                 Serial.println(settingsContent->settings->variant["cfgVal"].as<int32_t>());
// #endif
//             }
//             else
//             {
//                 uint32_t value = editIntContent->getUnsignedInt();
// #if DEBUG == 1
//                 Serial.print("Saving unsigned int: ");
//                 Serial.println(value);
// #endif
//                 settingsContent->settings->variant["cfgVal"] = value;
// #if DEBUG == 1
//                 Serial.print("Unsigned int saved: ");
//                 Serial.println(settingsContent->settings->variant["cfgVal"].as<uint32_t>());
// #endif
//             }
//             settingsContent->popVariant();
//             this->saveSettings = true;
//             content = settingsContent;
//         }
//         else if (content->type == ContentType::EDIT_FLOAT)
//         {
//             double value = editFloatContent->getFloat();
//             settingsContent->settings->variant.as<ArduinoJson::JsonObject>()["cfgVal"] = value;
//             settingsContent->popVariant();
//             this->saveSettings = true;
//             content = settingsContent;
//         }
//         else if (content->type == ContentType::EDIT_STRING)
//         {
//             const char *value = editStringContent->getString();
//             size_t len = strlen(value);
//             char *tmpString = new char[len + 1];
//             strcpy(tmpString, value);
//             tmpString[len] = '\0';
//             settingsContent->settings->variant["cfgVal"] = (const char *)tmpString;
// #if DEBUG == 1
//             Serial.print("value: ");
//             Serial.println(value);
//             Serial.print("String saved: ");
//             Serial.println(tmpString);
//             Serial.print("String saved: ");
//             Serial.println(settingsContent->settings->variant["cfgVal"].as<const char *>());
// #endif
//             settingsContent->popVariant();

//             // Settings_Manager::writeSettingsToEEPROM();
//             this->saveSettings = true;
//             editStringContent->stop();
//             Settings_Manager::settings.garbageCollect();
//             content = settingsContent;
//             this->settingsContent->refresh();
//         }
//         else if (content->type == ContentType::EDIT_ENUM)
//         {
//             size_t value = editEnumContent->getSelectedIndex();
//             settingsContent->settings->variant.as<ArduinoJson::JsonObject>()["cfgVal"] = value;
//             settingsContent->popVariant();
//             this->saveSettings = true;
//             content = settingsContent;
//         }
//         break;
//     case BUTTON_3:
//     {
//         callbackID = btn3CallbackID;
//         if (content->type == ContentType::SETTINGS)
//         {
//             if (settingsContent->settings->next == NULL)
//             {
//                 if (this->saveSettings)
//                 {
//                     Settings_Manager::writeSettingsToEEPROM();
//                     this->saveSettings = false;
//                     OLED_Manager::rebootDevice(inputID);
//                 }
//                 OLED_Manager::goBack(inputID);
//             }
//             else
//             {
//                 settingsContent->popVariant();
//             }
//         }
//         else if (content->type == ContentType::EDIT_INT ||
//                  content->type == ContentType::EDIT_STRING ||
//                  content->type == ContentType::EDIT_FLOAT ||
//                  content->type == ContentType::EDIT_ENUM)
//         {
// #if DEBUG == 1
//             Serial.println("Exiting edit window");
// #endif
//             if (content->type == ContentType::EDIT_STRING)
//             {
//                 editStringContent->stop();
//             }

//             settingsContent->popVariant();
//             content = settingsContent;
//         }
//     }
//     break;
//     case BUTTON_4:
//         callbackID = btn4CallbackID;
//         if (content->type == ContentType::SETTINGS)
//         {
//             if (settingsContent->settings->type == JSON_VARIANT_TYPE_OBJECT)
//             {
//                 ArduinoJson::JsonObject::iterator it = settingsContent->settings->variant.as<ArduinoJson::JsonObject>().begin();
//                 it += settingsContent->settingsIndex;
//                 JsonVariant variant = it->value();
//                 JSON_VARIANT_TYPE type = settingsContent->getVariantType(variant);
//                 if (type == JSON_VARIANT_CONFIGURABLE_INTEGER)
//                 {
//                     bool isSigned = variant["signed"].as<bool>();
//                     if (isSigned)
//                     {
//                         int32_t value = variant["cfgVal"].as<int32_t>();
//                         int32_t min = variant["minVal"].as<int32_t>();
//                         int32_t max = variant["maxVal"].as<int32_t>();
//                         uint32_t increment = variant["incVal"].as<uint32_t>();
//                         editIntContent->editSignedInt(value, min, max, increment);
//                     }
//                     else
//                     {
//                         uint32_t value = variant["cfgVal"].as<uint32_t>();
//                         uint32_t min = variant["minVal"].as<uint32_t>();
//                         uint32_t max = variant["maxVal"].as<uint32_t>();
//                         uint32_t increment = variant["incVal"].as<uint32_t>();
//                         editIntContent->editUnsignedInt(value, min, max, increment);
//                     }

// #if DEBUG == 1
//                     Serial.println("Swapping to edit int content");
// #endif
//                     this->content = editIntContent;
//                     // this->settingsContent->pushVariant(variant);
//                 }
//                 else if (type == JSON_VARIANT_CONFIGURABLE_STRING)
//                 {
//                     const char *value = variant["cfgVal"].as<const char *>();
//                     size_t maxLength = variant["maxLen"].as<size_t>();
//                     size_t strPos = strlen(value);
//                     editStringContent->setString(value, maxLength, strPos);

//                     this->content = editStringContent;
//                     editStringContent->start();
//                 }
//                 else if (type == JSON_VARIANT_CONFIGURABLE_FLOAT)
//                 {
//                     double value = variant["cfgVal"].as<double>();
//                     double min = variant["minVal"].as<double>();
//                     double max = variant["maxVal"].as<double>();
//                     double increment = variant["incVal"].as<double>();
//                     editFloatContent->editFloat(value, min, max, increment);

//                     this->content = editFloatContent;
//                 }
//                 else if (type == JSON_VARIANT_CONFIGURABLE_ENUM)
//                 {
//                     editEnumContent->assignEnum(variant["vals"].as<JsonArray>(), variant["valTxt"].as<JsonArray>(), variant["cfgVal"].as<size_t>());
//                     this->content = editEnumContent;
//                 }
//                 else
//                 {
//                     // content->pushVariant(variant);
//                 }
//                 settingsContent->pushVariant(it->value());
//             }
//             else if (settingsContent->settings->type == JSON_VARIANT_TYPE_ARRAY)
//             {
//                 ArduinoJson::JsonArray::iterator it = settingsContent->settings->variant.as<ArduinoJson::JsonArray>().begin();
//                 it += settingsContent->settingsIndex;
//                 settingsContent->pushVariant(*it);
//             }
//         }
//         else if (content->type == ContentType::EDIT_STRING)
//         {
//             editStringContent->passButtonPress(BUTTON_4);
//         }

//         break;
//     default:
//         break;
//     }

//     switch (callbackID)
//     {
//     case ACTION_BACK:
//         break;
//     default:
//         break;
//     }

//     this->drawWindow();
// }

// void Settings_Window::drawWindow()
// {
// #if DEBUG == 1
//     Serial.println("Settings_Window::drawWindow()");
// #endif
//     display->clearDisplay();
//     display->setTextSize(1);
//     display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);

//     if (this->content->type == ContentType::SETTINGS)
//     {
// #if DEBUG == 1
//         Serial.println("Drawing settings content");
// #endif
//         display->setCursor(0, OLED_HEIGHT - 8);
//         if (settingsContent->getVariantDepth() == 0)
//         {
//             display->print("Back");
//         }
//         else
//         {
//             display->print("<--");
//         }

//         if (settingsContent->settings->type == JSON_VARIANT_TYPE_OBJECT)
//         {
//             ArduinoJson::JsonObject::iterator it = settingsContent->settings->variant.as<ArduinoJson::JsonObject>().begin();
//             it += settingsContent->settingsIndex;
//             auto type = settingsContent->getVariantType(it->value());
//             if (type == JSON_VARIANT_CONFIGURABLE_BOOL ||
//                 type == JSON_VARIANT_CONFIGURABLE_INTEGER ||
//                 type == JSON_VARIANT_CONFIGURABLE_STRING ||
//                 type == JSON_VARIANT_CONFIGURABLE_FLOAT ||
//                 type == JSON_VARIANT_CONFIGURABLE_ENUM)
//             {
//                 display->setCursor(OLED_WIDTH - 24, OLED_HEIGHT - 8);
//                 display->print("Edit");
//             }
//             else
//             {
//                 display->setCursor(OLED_WIDTH - 36, OLED_HEIGHT - 8);
//                 display->print("Select");
//             }
//         }
//     }
//     else if (this->content->type == ContentType::EDIT_BOOL)
//     {
//     }
//     else if (this->content->type == ContentType::EDIT_INT)
//     {
// #if DEBUG == 1
//         Serial.println("Drawing edit int content");
// #endif
//         display->setCursor(OLED_WIDTH - 24, 0);
//         display->print("Save");
//         display->setCursor(0, OLED_HEIGHT - 8);
//         display->print("Back");
//     }
//     else if (this->content->type == ContentType::EDIT_STRING)
//     {
//         display->fillRect(0, 0, OLED_WIDTH, 8, SSD1306_BLACK);
//         display->fillRect(0, OLED_HEIGHT - 8, OLED_WIDTH, 8, SSD1306_BLACK);
//         display->setCursor(0, 0);
//         display->print("Backspace");
//         display->setCursor(OLED_WIDTH - 36, OLED_HEIGHT - 8);
//         display->print("Select");
//         display->setCursor(OLED_WIDTH - 24, 0);
//         display->print("Save");
//         display->setCursor(0, OLED_HEIGHT - 8);
//         display->print("Back");
//     }
//     else if (this->content->type == ContentType::EDIT_FLOAT)
//     {
//         display->setCursor(OLED_WIDTH - 24, 0);
//         display->print("Save");
//         display->setCursor(0, OLED_HEIGHT - 8);
//         display->print("Back");
//     }
//     else if (this->content->type == ContentType::EDIT_ENUM)
//     {
//         display->setCursor(OLED_WIDTH - 24, 0);
//         display->print("Save");
//         display->setCursor(0, OLED_HEIGHT - 8);
//         display->print("Back");
//     }

//     if (content != NULL)
//     {
// #if DEBUG == 1
//         Serial.println("Settings_Window::drawWindow() printing content");
// #endif
//         content->printContent();
//     }
//     else
//         display->display();
// }

/*
void Settings_Window::handleBtnInterrupt(uint8_t buttonNumber, void *arg, OLED_Window *window)
{
#if DEBUG == 1
    Serial.print("void Settings_Window::handleBtnInterrupt(uint8_t buttonNumber, void *arg): Button number: ");
    Serial.println(buttonNumber);
#endif
    Settings_Window *thisWindow = (Settings_Window *)window;
    Settings_Content *content = (Settings_Content *)thisWindow->content;
    switch (buttonNumber)
    {
    // back button
    case BUTTON_1:
        if (content->settings->next == NULL)
        {
            OLED_Manager::goBack(NULL);
        }
        else
        {
            content->popVariant();
        }
        break;
    // select button
    case BUTTON_2:
        if (content->settings->type == JSON_VARIANT_TYPE_OBJECT)
        {
            ArduinoJson::JsonObject::iterator it = content->settings->variant.as<ArduinoJson::JsonObject>().begin();
            it += content->settingsIndex;
            content->pushVariant(it->value());
        }
        else if (content->settings->type == JSON_VARIANT_TYPE_ARRAY)
        {
            ArduinoJson::JsonArray::iterator it = content->settings->variant.as<ArduinoJson::JsonArray>().begin();
            it += content->settingsIndex;
            content->pushVariant(*it);
        }
        break;
    default:
        break;
    }
}
*/