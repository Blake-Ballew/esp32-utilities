#pragma once

#include "Window_State.h"
#include "Saved_Messages_Content.h"
#include "ScrollWheel.h"
#include "LED_Utils.h"
#include <vector>
#include <string>

namespace
{
    const char *MSG_SELECT PROGMEM = "Select a message";
    const char *NO_MSG PROGMEM = "No messages";
}

/*
    Takes in a set of messages to be sent as a broadcast or direct message.
    The user can select a message to send from a list of saved messages or
    use the name of the location as the message.
*/

class Select_Message_State : public Window_State
{
public:
    // edit bool decides if state is used to manage saved messages or send them.
    Select_Message_State()
    {
        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Send");
    }

    ~Select_Message_State()
    {

    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        _ScrollWheelPatternID = ScrollWheel::RegisteredPatternID();
        LED_Utils::enablePattern(_ScrollWheelPatternID);

        _Messages.clear();

        if (transferData.serializedData != nullptr)
        {
            DynamicJsonDocument *doc = transferData.serializedData;
            if (doc->containsKey("Messages") && (*doc)["Messages"].is<JsonArray>())
            {
                for (auto msg : (*doc)["Messages"].as<JsonArray>())
                {
                    _Messages.push_back(msg.as<std::string>());
                }

                _MessageIt = _Messages.begin();
            }
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        LED_Utils::disablePattern(_ScrollWheelPatternID);

        if (_MessageIt != _Messages.end() && transferData.inputID == BUTTON_4)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(200);
            (*doc)["message"] = *_MessageIt;
            transferData.serializedData = doc;
        }
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
        {
            if (_Messages.size() > 0)
            {
                if (_MessageIt == _Messages.begin())
                {
                    _MessageIt = _Messages.end() - 1;
                }
                else
                {
                    _MessageIt--;
                }
            }
        }
        break;
        case ENC_DOWN:
        {
            if (_Messages.size() > 0)
            {
                _MessageIt++;

                if (_MessageIt == _Messages.end())
                {
                    _MessageIt = _Messages.begin();
                }
            }
        }
        break;
        default:
            break;
        }
    }

    void displayState()
    {
        if (_Messages.size() > 0)
        {

            if (_ScrollWheelPatternID > -1)
            {
                StaticJsonDocument<128> doc;

                doc["numItems"] = _Messages.size();
                doc["currItem"] = std::distance(_Messages.begin(), _MessageIt);
                LED_Utils::configurePattern(_ScrollWheelPatternID, doc);
                LED_Utils::iteratePattern(_ScrollWheelPatternID);
            }

            TextFormat prompt;
            prompt.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            prompt.verticalAlignment = TEXT_LINE;
            prompt.line = 2;

            Display_Utils::printFormattedText(MSG_SELECT, prompt);

            TextFormat msg;
            msg.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            msg.verticalAlignment = TEXT_LINE;
            msg.line = 3;

            Display_Utils::printFormattedText(_MessageIt->c_str(), msg);
        }
        else
        {
            TextFormat prompt;
            prompt.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            prompt.verticalAlignment = ALIGN_CENTER_VERTICAL;

            Display_Utils::printFormattedText(NO_MSG, prompt);
        }

        display->display();
    }

private:
    
    std::vector<std::string> _Messages;
    std::vector<std::string>::iterator _MessageIt;
    uint64_t userID;

    int _ScrollWheelPatternID;
};
