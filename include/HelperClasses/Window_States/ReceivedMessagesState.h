#pragma once

#include "Window_State.h"
#include "MessageBase.h"
#include "LoraUtils.h"
#include "SolidRing.h"
#include "Display_Utils.h"
#include "LoraMessageDisplay.h"


class ReceivedMessagesState : public Window_State
{
public:
    ReceivedMessagesState(LoraMessageDisplay *loraDisplay) 
    {
        messageDisplay = loraDisplay;
        renderContent = messageDisplay;

        assignInput(BUTTON_3, ACTION_BACK, "Back");

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

    ~ReceivedMessagesState() {}

    void processInput(uint8_t inputID) 
    {
        bool updateMessage = false;

        if (inputID == ENC_UP)
        {
            if (LoraUtils::IsMessageIteratorAtBeginning())
            {
                Display_Utils::sendCallbackCommand(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE);
                return;
            }
            
            LoraUtils::DecrementMessageIterator();
            updateMessage = true;
        }
        else if (inputID == ENC_DOWN)
        {
            LoraUtils::IncrementMessageIterator();
            updateMessage = true;

            if (LoraUtils::IsMessageIteratorAtEnd())
            {
                LoraUtils::DecrementMessageIterator();
                updateMessage = false;
            }
        }
        
        if (updateMessage)
        {
            MessageBase *msg = LoraUtils::GetCurrentMessage();
            if (msg != nullptr)
            {
                messageDisplay->SetDisplayMessage(msg);
            }
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        ESP_LOGI(TAG, "enterState");
        if (renderContent != nullptr)
        {
            renderContent->start();
        }

        LoraUtils::ResetMessageIterator();

        MessageBase *msg = LoraUtils::GetCurrentMessage();
        if (msg != nullptr)
        {
            messageDisplay->SetDisplayMessage(msg);
        }

        ESP_LOGI(TAG, "enterState - Done");
    }

    void exitState(State_Transfer_Data &transferData)
    {
        if (renderContent != nullptr)
        {
            renderContent->stop();
        }

        LED_Utils::clearPattern(_SolidRingPatternID);

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
    }

    void displayState()
    {
        if (LoraUtils::GetNumMessages() == 0)
        {
            Display_Utils::printCenteredText("No messages");
        }
        else
        {
            MessageBase *msg = LoraUtils::GetCurrentMessage();
            if (msg != nullptr)
            {
                messageDisplay->SetDisplayMessage(msg);
            }
            else
            {
                ESP_LOGW(TAG, "displayState - Message is null");
            }

            if (messageDisplay->DisplayMessage()->GetInstanceMessageType() == MessagePing::MessageType())
            {
                MessagePing *ping = (MessagePing *)messageDisplay->DisplayMessage();

                StaticJsonDocument<128> doc;
                doc["rOverride"] = ping->color_R;
                doc["gOverride"] = ping->color_G;
                doc["bOverride"] = ping->color_B;

                LED_Utils::configurePattern(_SolidRingPatternID, doc);
                LED_Utils::iteratePattern(_SolidRingPatternID);
            }
        }

        Window_State::displayState();
    }

protected:
    LoraMessageDisplay *messageDisplay;
    int _SolidRingPatternID;
};