#pragma once

#include "Window_State.h"
#include "LoraUtils.h"

class DisplaySentMessageState : public Window_State
{
public:
    DisplaySentMessageState()
    {
        _DisplayMessage = nullptr;
    }

    ~DisplaySentMessageState()
    {
        if (_DisplayMessage != nullptr)
        {
            delete _DisplayMessage;
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        if (_DisplayMessage != nullptr)
        {
            delete _DisplayMessage;
        }

        _DisplayMessage = LoraUtils::MyLastBroacast();

        if (_DisplayMessage == nullptr)
        {
            buttonCallbacks.erase(BUTTON_4);
        }
        else
        {
            assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Retransmit");
        }

        Window_State::enterState(transferData);
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);
        
        // Retransmitting message
        if (transferData.callbackID == ACTION_CALL_FUNCTIONAL_WINDOW_STATE)
        {
            if (_DisplayMessage != nullptr)
            {
                DynamicJsonDocument *doc = new DynamicJsonDocument(512);

                _DisplayMessage->serialize(*doc);

                transferData.serializedData = doc;

                #if DEBUG == 1
                Serial.print("Retransmitting message: ");
                serializeJson(*transferData.serializedData, Serial);
                Serial.println();
                #endif
            }
        }

        delete _DisplayMessage;
        _DisplayMessage = nullptr;
    }

    void displayState()
    {
        if (_DisplayMessage != nullptr)
        {
            std::vector<MessagePrintInformation> displayInfo;
            _DisplayMessage->GetPrintableInformation(displayInfo);

            TextFormat format;
            format.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            format.verticalAlignment = TEXT_LINE;
            format.line = 1;

            Display_Utils::printFormattedText("My Current Status", format);
            format.line++;

            for (auto &info : displayInfo)
            {
                Display_Utils::printFormattedText(info.txt, format);
                format.line++;
            } 
        }
        else
        {
            Display_Utils::printCenteredText("No status sent");
        }

        Display_Utils::UpdateDisplay().Invoke();
    }

protected:
    // Message copied from LoRaUtils. This state is responsible for deleting it
    MessageBase *_DisplayMessage;
};