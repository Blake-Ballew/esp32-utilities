#pragma once

#include "globalDefines.h"
#include "Window_State.h"
#include "LoraUtils.h"
#include "MessagePing.h"
#include "Repeat_Message_Content.h"
#include "Text_Display_Content.h"
#include "Ring_Pulse.h"
#include "LED_Utils.h"

#define MESSAGE_REPEAT_INTERVAL_MS 30000

class Repeat_Message_State : public Window_State
{
public:
    Repeat_Message_State(Text_Display_Content *content, bool newMsgID = false)
    {
        allowInterrupts = false;

        this->newMsgID = newMsgID;
        message = nullptr;
        assignInput(BUTTON_3, ACTION_BACK, "Back");
        textContent = content;
        renderContent = textContent;

        ringPulseID = Ring_Pulse::RegisteredPatternID();
    }

    ~Repeat_Message_State()
    {
        if (message != nullptr)
        {
            delete message;
        }

        LED_Utils::disablePattern(ringPulseID);
        LED_Utils::loopPattern(ringPulseID, 0);
        LED_Utils::resetPattern(ringPulseID);
    }

    void enterState(State_Transfer_Data &transferData)
    {
        ESP_LOGI(TAG, "Enter Repeat Message State");
        Window_State::enterState(transferData);
        
        ringPulseID = Ring_Pulse::RegisteredPatternID();
        
        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            auto messageType = MessageBase::GetMessageTypeFromJson(*doc);

            if (message != nullptr)
            {
                delete message;
                message = nullptr;
            }

            if (messageType == MessagePing::MessageType())
            {
                ESP_LOGD(TAG, "enterState: MessagePing");
                MessagePing *ping = new MessagePing();
                ping->deserialize(*doc);
                message = ping;

                uint8_t msgR = ping->color_R;
                uint8_t msgG = ping->color_G;
                uint8_t msgB = ping->color_B;

                StaticJsonDocument<200> cfg;
                cfg["rOverride"] = msgR;
                cfg["gOverride"] = msgG;
                cfg["bOverride"] = msgB;

                ESP_LOGD(TAG, "RingPulseID: %d", ringPulseID);

                LED_Utils::setAnimationLengthMS(ringPulseID, 3000);
                LED_Utils::configurePattern(ringPulseID, cfg);

                LED_Utils::enablePattern(ringPulseID);
                LED_Utils::loopPattern(ringPulseID, -1);                
            }

            if (message != nullptr)
            {
                Display_Utils::enableRefreshTimer(MESSAGE_REPEAT_INTERVAL_MS);
            }
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        LED_Utils::disablePattern(ringPulseID);
        LED_Utils::loopPattern(ringPulseID, 0);
        LED_Utils::resetPattern(ringPulseID);

        if (message != nullptr)
        {
            delete message;
            message = nullptr;
        }

        Display_Utils::disableRefreshTimer();
    }

    void displayState()
    {
        Window_State::displayState();

        if (message != nullptr)
        {
            if (newMsgID)
            {
                message->msgID = esp_random();
            }

            if (message->GetInstanceMessageType() == MessagePing::MessageType())
            {
                MessagePing *ping = (MessagePing *)message;

                if (ping->IsLive)
                {
                    ESP_LOGD(TAG, "displayState: Updating GPS");
                    NavigationUtils::UpdateGPS();

                    ping->lat = NavigationUtils::GetLocation().lat();
                    ping->lng = NavigationUtils::GetLocation().lng();
                    ping->time = NavigationUtils::GetTime().value();
                    ping->date = NavigationUtils::GetDate().value();
                }
            }

            LoraUtils::SendMessage(message, 1);
        }
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
    MessageBase *message;
    Text_Display_Content *textContent;

    int ringPulseID;
    bool newMsgID;
};
