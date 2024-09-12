#pragma once

#include "Window_State.h"
#include "MessageBase.h"
#include "LoraUtils.h"
#include "SolidRing.h"
#include "Display_Utils.h"
#include "LoraMessageDisplay.h"

class UnreadMessageState : public Window_State
{
public:
    UnreadMessageState(LoraMessageDisplay *loraDisplay) 
    {
        messageDisplay = loraDisplay;
        renderContent = messageDisplay;

        if (SolidRing::RegisteredPatternID() == -1)
        {
            SolidRing *ring = new SolidRing();
            _SolidRingPatternID = LED_Utils::registerPattern(ring);

            StaticJsonDocument<128> doc;

            doc["beginIdx"] = 0;
            doc["endIdx"] = 15;
            LED_Utils::configurePattern(_SolidRingPatternID, doc);
        }
        else
        {
            _SolidRingPatternID = SolidRing::RegisteredPatternID();
        }

        LED_Utils::enablePattern(_SolidRingPatternID);
    }

    ~UnreadMessageState() {}

    void processInput(uint8_t inputID) 
    {
        bool updateMessage = false;

        if (inputID == ENC_UP)
        {
            if (LoraUtils::IsUnreadMessageIteratorAtBeginning())
            {
                Display_Utils::sendCallbackCommand(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE);
                return;
            }
            
            LoraUtils::DecrementUnreadMessageIterator();
            updateMessage = true;
        }
        else if (inputID == ENC_DOWN)
        {
            LoraUtils::IncrementUnreadMessageIterator();
            updateMessage = true;

            if (LoraUtils::IsUnreadMessageIteratorAtEnd())
            {
                LoraUtils::DecrementUnreadMessageIterator();
                updateMessage = false;
            }
        }
        else if (inputID == BUTTON_3)
        {
            LoraUtils::MarkMessageOpened(LoraUtils::GetCurrentUnreadMessageSenderID());
            updateMessage = true;
            
            if (LoraUtils::IsUnreadMessageIteratorAtEnd())
            {
                if (LoraUtils::GetNumUnreadMessages() == 0)
                {
                    Display_Utils::sendCallbackCommand(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE);
                    return;
                }
                else
                {
                    LoraUtils::DecrementUnreadMessageIterator();
                }
            }
        }
        
        if (updateMessage)
        {
            MessageBase *msg = LoraUtils::GetCurrentUnreadMessage();
            if (msg != nullptr)
            {
                messageDisplay->SetDisplayMessage(msg);
            }
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("UnreadMessageState::enterState()");
#endif
        if (renderContent != nullptr)
        {
            renderContent->start();
        }

        LoraUtils::ResetUnreadMessageIterator();

        MessageBase *msg = LoraUtils::GetCurrentUnreadMessage();
        if (msg != nullptr)
        {
            messageDisplay->SetDisplayMessage(msg);
        }

        LED_Utils::enablePattern(_SolidRingPatternID);

        #if DEBUG == 1
        Serial.println("UnreadMessageState::enterState() - Done");
        #endif
    }

    void exitState(State_Transfer_Data &transferData)
    {
        if (renderContent != nullptr)
        {
            renderContent->stop();
        }

        switch (transferData.inputID)
        {
            // Track message. Pass serialized message data to tracking state.
            case BUTTON_4:
                {
                    auto msg = messageDisplay->DisplayMessage();
                    if (msg != nullptr && msg->GetInstanceMessageType() == MessagePing::MessageType())
                    {
                        DynamicJsonDocument *doc = new DynamicJsonDocument(512);
                        MessagePing *ping = (MessagePing *)msg;
                        std::vector<MessagePrintInformation> info;
                        
                        (*doc)["lat"] = ping->lat;
                        (*doc)["lon"] = ping->lng;

                        (*doc)["color_R"] = ping->color_R;
                        (*doc)["color_G"] = ping->color_G;
                        (*doc)["color_B"] = ping->color_B;

                        ping->GetPrintableInformation(info);

                        auto displayArray = doc->createNestedArray("displayTxt");

                        for (auto &i : info)
                        {
                            displayArray.add(i.txt);
                        }

                        transferData.serializedData = doc;
                    }
                    break;
                }
            case BUTTON_2:
                {
                    auto msg = messageDisplay->DisplayMessage();
                    if (msg != nullptr)
                    {
                        DynamicJsonDocument *doc = new DynamicJsonDocument(64);
                        (*doc)["recipientID"] = msg->sender;
                        transferData.serializedData = doc;
                    }
                    break;
                }

            // Reply to message. 
            default:
                break;
        }

        LED_Utils::disablePattern(_SolidRingPatternID);
        LED_Utils::clearPattern(_SolidRingPatternID);
    }

    void displayState()
    {
        Window_State::displayState();

        if (messageDisplay->DisplayMessage()->GetInstanceMessageType() == MessagePing::MessageType())
        {
            MessagePing *ping = (MessagePing *)messageDisplay->DisplayMessage();

            StaticJsonDocument<128> doc;
            doc["rOverride"] = ping->color_R;
            doc["gOverride"] = ping->color_G;
            doc["bOverride"] = ping->color_B;

            #if DEBUG == 1
            // Serial.println("LoraMessageDisplay::printContent(): Configuring SolidRing");
            #endif
            LED_Utils::configurePattern(_SolidRingPatternID, doc);

            #if DEBUG == 1
            // Serial.println("LoraMessageDisplay::printContent(): Iterating SolidRing");
            #endif
            LED_Utils::iteratePattern(_SolidRingPatternID);
        }
    }

protected:
    LoraMessageDisplay *messageDisplay;
    int _SolidRingPatternID;
};