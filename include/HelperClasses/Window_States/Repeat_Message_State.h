#pragma once

#include "globalDefines.h"
#include "Window_State.h"
#include "Network_Manager.h"
#include "Repeat_Message_Content.h"
#include "Text_Display_Content.h"
#include "Ring_Pulse.h"
#include "LED_Utils.h"

#define MESSAGE_REPEAT_INTERVAL_MS 15000

class Repeat_Message_State : public Window_State
{
public:
    Repeat_Message_State(Text_Display_Content *content,
        int beginLedIdx,
        int endLedIdx,
        bool newMsgID = false)
    {
        this->newMsgID = newMsgID;
        message = nullptr;
        assignInput(BUTTON_3, ACTION_BACK, "Back");
        textContent = content;
        renderContent = textContent;

        if (Ring_Pulse::RegisteredPatternID() == -1) 
        {
            Ring_Pulse *pulse = new Ring_Pulse();
            
            ringPulseID = LED_Utils::registerPattern(pulse);
        }
        else 
        {
            ringPulseID = Ring_Pulse::RegisteredPatternID();
        }

        StaticJsonDocument<200> doc;
        doc["beginIdx"] = beginLedIdx;
        doc["endIdx"] = endLedIdx;

        auto cfgObj = doc.as<JsonObject>();

        LED_Utils::configurePattern(ringPulseID, cfgObj);
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

            if (messageType == MESSAGE_PING) 
            {
                Message_Ping *ping = new Message_Ping();
                ping->deserialize(doc);
                message = ping;

                uint8_t msgR = ping->color_R;
                uint8_t msgG = ping->color_G;
                uint8_t msgB = ping->color_B;

                StaticJsonDocument<200> cfg;
                cfg["rOverride"] = msgR;
                cfg["gOverride"] = msgG;
                cfg["bOverride"] = msgB;

                auto cfgObj = cfg.as<JsonObject>();

                LED_Utils::configurePattern(ringPulseID, cfgObj);
            }

            if (message != nullptr)
            {
                Display_Utils::enableRefreshTimer(MESSAGE_REPEAT_INTERVAL_MS);
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

        Display_Utils::disableRefreshTimer();
    }

    void PauseState()
    {
        LED_Utils::clearPattern(ringPulseID);
    }

    void ResumeState()
    {
        LED_Utils::loopPattern(ringPulseID, -1);
    }
    
protected:
    Message_Base *message;
    Text_Display_Content *textContent;

    int ringPulseID;
    bool newMsgID;
};
