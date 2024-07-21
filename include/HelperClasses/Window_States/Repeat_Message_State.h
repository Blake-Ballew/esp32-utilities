#pragma once

#include "globalDefines.h"
#include "Window_State.h"
#include "Network_Manager.h"
#include "Repeat_Message_Content.h"

class Repeat_Message_State : public Window_State
{
public:
    Repeat_Message_State(Repeat_Message_Content *content)
    {
        message = nullptr;
        assignInput(BUTTON_3, ACTION_BACK, "Back");
        repeatContent = content;
        renderContent = repeatContent;
    }

    ~Repeat_Message_State()
    {
        if (message != nullptr)
        {
            delete message;
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);
        
        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            auto messageType = Message_Base::getMessageType(doc);

            if (message != nullptr)
            {
                delete message;
                message = nullptr;
            }

            switch (messageType) {
                case MessageType::MESSAGE_BASE: {
                    message = new Message_Base();
                    break;
                }
                case MessageType::MESSAGE_PING: {
                    message = new Message_Ping();
                    break;
                }
            }

            if (message != nullptr)
            {
                message->deserialize(*doc);
            }
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        if (message != nullptr)
        {
            delete message;
            message = nullptr;
        }
    }
    
protected:
    Message_Base *message;
    Repeat_Message_Content *repeatContent;
};
