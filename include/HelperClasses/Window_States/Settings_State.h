#pragma once

#include "globalDefines.h"
#include "Window_State.h"
#include "Settings_Content.h"
#include "Settings_Manager.h"
#include "FilesystemUtils.h"

class Settings_State : public Window_State
{
public:
    Settings_State()
    {
        settingsContent = new Settings_Content(FilesystemModule::Utilities::SettingsFile());
        renderContent = settingsContent;

        strcpy(upOneLevel.displayText, "<-");
        upOneLevel.callbackID = ACTION_DEFER_CALLBACK_TO_WINDOW;

        strcpy(selectItem.displayText, "Select");
        selectItem.callbackID = ACTION_DEFER_CALLBACK_TO_WINDOW;

        strcpy(exitSettings.displayText, "Back");
        exitSettings.callbackID = ACTION_DEFER_CALLBACK_TO_WINDOW;

        strcpy(editItem.displayText, "Edit");
        editItem.callbackID = ACTION_CALL_FUNCTIONAL_WINDOW_STATE;

        strcpy(toggleBool.displayText, "Toggle");
        toggleBool.callbackID = ACTION_DEFER_CALLBACK_TO_WINDOW;

        assignInput(BUTTON_3, exitSettings);

        assignInput(ENC_UP, ACTION_DEFER_CALLBACK_TO_WINDOW);
        assignInput(ENC_DOWN, ACTION_DEFER_CALLBACK_TO_WINDOW);
    }

    ~Settings_State()
    {
        if (settingsContent != nullptr)
        {
            settingsContent->stop();
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        if (settingsContent != nullptr)
        {
            settingsContent->start();
        }

        if (transferData.serializedData != nullptr && transferData.serializedData->containsKey("return"))
        {
            auto returnData = (*transferData.serializedData)["return"].as<JsonVariant>();
            settingsContent->saveReturnValueFromEditState(returnData);
            settingsSaved = true;
        }

        if (settingsContent->getVariantDepth() > 0)
        {
            settingsContent->popVariant();
        }

        updateInputs();
    }

    void exitState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Settings_State::exitState");
#endif
        if (settingsContent != nullptr)
        {
            settingsContent->stop();
        }

        // if (transferData.callbackID == ACTION_CALL_FUNCTIONAL_WINDOW_STATE)
        // {
        //     transferData.serializedData = settingsContent->getEditStateInput();
        // }
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
            settingsContent->encUp();
            updateInputs();
            break;
        case ENC_DOWN:
            settingsContent->encDown();
            updateInputs();
            break;
        case BUTTON_3:
        {
            if (buttonCallbacks.find(BUTTON_3) != buttonCallbacks.end() &&
                buttonCallbacks[BUTTON_3].callbackID == ACTION_DEFER_CALLBACK_TO_WINDOW && settingsContent->getVariantDepth() > 0)
            {
                settingsContent->popVariant();
                updateInputs();
            }
            else if (buttonCallbacks.find(BUTTON_3) != buttonCallbacks.end() &&
                     buttonCallbacks[BUTTON_3].callbackID == ACTION_DEFER_CALLBACK_TO_WINDOW && settingsContent->getVariantDepth() == 0)
            {
                // save settings if any have changed
                if (settingsSaved)
                {
                    FilesystemModule::Utilities::WriteSettingsFileToFlash();
                    settingsSaved = false;
                }

                Display_Utils::sendCallbackCommand(ACTION_BACK);
            }
            break;
        }
        case BUTTON_4:
        {
            
            if (buttonCallbacks.find(BUTTON_4) != buttonCallbacks.end())
            {
                if (buttonCallbacks[BUTTON_4] == toggleBool)
                {
                    auto boolObj = settingsContent->getSelectionVariant();
                    boolObj.set(!boolObj.as<bool>());
                    settingsSaved = true;
                }
                else if (buttonCallbacks[BUTTON_4].callbackID == ACTION_DEFER_CALLBACK_TO_WINDOW)
                {
                    settingsContent->pushVariant();
                    updateInputs();
                }
            }
            break;
        }
        default:
            break;
        }
    }

    JsonVariant getCurrentVariant()
    {
        return settingsContent->getCurrentVariant();
    }

    DynamicJsonDocument *initializeEdit(bool &success, JsonVariantType &selectedType)
    {
#if DEBUG == 1
        Serial.println("Settings_State::initializeEdit");
#endif
        selectedType = settingsContent->getSelectionVariantType();
        #if DEBUG == 1
        Serial.print("Settings_State::initializeEdit: selectedType: ");
        Serial.println(selectedType);
#endif

        if (selectedType == JSON_VARIANT_CONFIGURABLE_STRING ||
            selectedType == JSON_VARIANT_CONFIGURABLE_ENUM ||
            selectedType == JSON_VARIANT_CONFIGURABLE_FLOAT ||
            selectedType == JSON_VARIANT_CONFIGURABLE_INTEGER ||
            selectedType == JSON_VARIANT_CONFIGURABLE_BOOL)
        {
            settingsContent->pushVariant();
            auto returnData = settingsContent->getEditStateInput();
#if DEBUG == 1
            if (returnData != nullptr)
            {
                Serial.println("Settings_State::initializeEdit: returnData");
                ArduinoJson::serializeJsonPretty(*returnData, Serial);
            }
#endif
            if (returnData != nullptr)
            {
                success = true;
                return returnData;
            }
        }

        success = false;
        return nullptr;
    }

protected:
    Settings_Content *settingsContent;

    bool settingsSaved = false;

    CallbackData upOneLevel;
    CallbackData selectItem;
    CallbackData exitSettings;
    CallbackData editItem;
    CallbackData toggleBool;

    void updateInputs()
    {
        auto currentVariantType = settingsContent->getVariantType();

        if (currentVariantType == JsonVariantType::JSON_VARIANT_TYPE_OBJECT ||
            currentVariantType == JsonVariantType::JSON_VARIANT_TYPE_ARRAY)
        {
            auto selectionVarType = settingsContent->getSelectionVariantType();

            if (selectionVarType == JsonVariantType::JSON_VARIANT_CONFIGURABLE_BOOL ||
                selectionVarType == JsonVariantType::JSON_VARIANT_CONFIGURABLE_INTEGER ||
                selectionVarType == JsonVariantType::JSON_VARIANT_CONFIGURABLE_FLOAT ||
                selectionVarType == JsonVariantType::JSON_VARIANT_CONFIGURABLE_STRING ||
                selectionVarType == JsonVariantType::JSON_VARIANT_CONFIGURABLE_ENUM)
            {
                assignInput(BUTTON_4, editItem);
            }
            else if (selectionVarType == JsonVariantType::JSON_VARIANT_TYPE_BOOLEAN)
            {
                assignInput(BUTTON_4, toggleBool);
            }
            else
            {
                assignInput(BUTTON_4, selectItem);
            }
        }
        else
        {
            buttonCallbacks.erase(BUTTON_4);
        }

        if (settingsContent->getVariantDepth() > 0)
        {
            assignInput(BUTTON_3, upOneLevel);
        }
        else
        {
            assignInput(BUTTON_3, exitSettings);
        }
    }
};