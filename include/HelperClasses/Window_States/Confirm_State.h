#pragma once

#include "Window_State.h"
#include "globalDefines.h"
#include <ArduinoJson.h>
#include <map>

namespace
{
    const int CONFIRM_PROMPT_LENGTH = 22;
}

class Confirm_State : public Window_State
{
public:
    Confirm_State()
    {
        // typeID = __COUNTER__;
        CallbackData noBtn;
        noBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(noBtn.displayText, "No");
        this->buttonCallbacks[BUTTON_3] = noBtn;

        CallbackData yesBtn;
        yesBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(yesBtn.displayText, "Yes");
        this->buttonCallbacks[BUTTON_4] = yesBtn;
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case BUTTON_3:
            confirmState = false;
            break;
        case BUTTON_4:
            confirmState = true;
            break;
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);
        confirmState = false;

        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            if (doc->containsKey("confirmPrompt"))
            {
                strncpy(confirmPrompt, (*doc)["confirmPrompt"], CONFIRM_PROMPT_LENGTH);
                confirmPrompt[strlen((*doc)["confirmPrompt"])] = '\0';
            }
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        ArduinoJson::DynamicJsonDocument *doc = new ArduinoJson::DynamicJsonDocument(64);
        (*doc)["return"] = confirmState;

        transferData.serializedData = doc;
    }

private:
    bool confirmState = false;
    char confirmPrompt[CONFIRM_PROMPT_LENGTH];
};