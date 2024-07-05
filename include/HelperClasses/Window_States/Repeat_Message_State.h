#pragma once

#include "globalDefines.h"
#include "Window_State.h"
#include "Network_Manager.h"

class Repeat_Message_State : public Window_State
{
public:
    Repeat_Message_State()
    {
        assignInput(BUTTON_3, ACTION_BACK, "Back");
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);
        
        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            Message_Base *message = Message_Base::getMessageType
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
    }

    bool confirmState = false;
    char confirmPrompt[CONFIRM_PROMPT_LENGTH];
    
protected:
    
};
