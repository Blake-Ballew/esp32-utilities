#pragma once

#include "Edit_States.h"
#include "LoraUtils.h"
#include "MessagePing.h"

class SaveStatusMessageState : public Edit_String_State
{
public:
    SaveStatusMessageState() : Edit_String_State()
    {

    }

    SaveStatusMessageState(Edit_String_Content *stringContent) : Edit_String_State(stringContent)
    {

    }

    void enterState(State_Transfer_Data &transferData)
    {
        ESP_LOGI(TAG, "SaveStatusMessageState::enterState");

        if (transferData.serializedData == nullptr)
        {
            transferData.serializedData = new DynamicJsonDocument(64);

            (*transferData.serializedData)["maxLen"] = STATUS_LENGTH;
        }

        Edit_String_State::enterState(transferData);
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Edit_String_State::exitState(transferData);

        // Only save location if the user confirmed a string
        if (transferData.serializedData != nullptr)
        {
            auto msgStr = (*transferData.serializedData)["return"].as<std::string>();

            LoraUtils::AddSavedMessage(msgStr);

            delete transferData.serializedData;
            transferData.serializedData = nullptr;
            
            display->clearDisplay();
            Display_Utils::printCenteredText("Status Message Saved");
            display->display();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }

};