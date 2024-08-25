#pragma once

#include "OLED_Content.h"
#include "MessageBase.h"

class LoraMessageDisplay : public OLED_Content
{
public:
    LoraMessageDisplay() 
    {
        _DisplayMessage = nullptr;
    }

    ~LoraMessageDisplay()
    {
        if (_DisplayMessage != nullptr)
        {
            delete _DisplayMessage;
        }
    }

    MessageBase *DisplayMessage() { return _DisplayMessage; }

    void SetDisplayMessage(MessageBase *message)
    {
        if (_DisplayMessage != nullptr)
        {
            delete _DisplayMessage;
        }

        _DisplayMessage = message;
    }

    void ClearDisplayMessage()
    {
        if (_DisplayMessage != nullptr)
        {
            delete _DisplayMessage;
            _DisplayMessage = nullptr;
        }
    }

    void printContent()
    {
        #if DEBUG == 1
        // Serial.println("LoraMessageDisplay::printContent()");
        #endif
        if (_DisplayMessage == nullptr)
        {
            return;
        }

        std::vector<MessagePrintInformation> displayInfo;
        _DisplayMessage->GetPrintableInformation(displayInfo);

        Display_Utils::clearContentArea();

        #if DEBUG == 1
        // Serial.printf("LoraMessageDisplay::printContent() displayInfo.size() = %d\n", displayInfo.size());
        #endif

        for (size_t i = 0; i < displayInfo.size(); i++)
        {
            // TODO: Define bounds of content area
            display->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(i + 2));
            display->print(displayInfo[i].txt);
        }

        // display->display();
    }

protected:
    MessageBase *_DisplayMessage;
};