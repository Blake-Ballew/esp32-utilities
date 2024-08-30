#pragma once

#include "Window_State.h"
#include "LoraUtils.h"
#include "ScrollWheel.h"
#include "LED_Utils.h"
#include "MessagePing.h"

// Displays a list of saved status messages
// Inputs:
//   ENC_UP: Select next message
//   ENC_DOWN: Select previous message
//   BUTTON_3: Return
//   BUTTON_4: Edit
//   BUTTON_1: Delete
//   BUTTON_2: Create new message
class SavedStatusMsgListState : public Window_State
{
public:
    SavedStatusMsgListState() : Window_State()
    {
        assignInput(ENC_UP, ACTION_DEFER_CALLBACK_TO_WINDOW);
        assignInput(ENC_DOWN, ACTION_DEFER_CALLBACK_TO_WINDOW);

        assignInput(BUTTON_3, ACTION_BACK, "Back");
        assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Edit");
        assignInput(BUTTON_1, ACTION_DEFER_CALLBACK_TO_WINDOW, "Delete");
        assignInput(BUTTON_2, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Create");
    }

    ~SavedStatusMsgListState()
    {

    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        _ScrollWheelPatternID = ScrollWheel::RegisteredPatternID();
        LED_Utils::enablePattern(_ScrollWheelPatternID);

        // Returned from edit state
        if (transferData.serializedData != nullptr && transferData.serializedData->containsKey("return"))
        {
            auto newStr = (*transferData.serializedData)["return"].as<std::string>();

            LoraUtils::UpdateSavedMessage(_SelectedMessageIt, newStr);
        }
        else
        {
            _SelectedMessageIt = LoraUtils::SavedMessageListBegin();
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        LED_Utils::disablePattern(_ScrollWheelPatternID);

        if (transferData.inputID == BUTTON_4)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(128);

            (*doc)["cfgVal"] = *_SelectedMessageIt;
            (*doc)["maxLen"] = STATUS_LENGTH;

            transferData.serializedData = doc;
        }
        else if (transferData.inputID == BUTTON_2)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(128);
            (*doc)["maxLen"] = STATUS_LENGTH;

            transferData.serializedData = doc;
        }
    }

    void processInput(uint8_t inputID)
    {
        if (inputID == ENC_DOWN)
        {
            _SelectedMessageIt++;

            if (_SelectedMessageIt == LoraUtils::SavedMessageListEnd())
            {
                _SelectedMessageIt = LoraUtils::SavedMessageListBegin();
            }
        }

        if (inputID == ENC_UP)
        {
            if (_SelectedMessageIt == LoraUtils::SavedMessageListBegin())
            {
                _SelectedMessageIt = LoraUtils::SavedMessageListEnd();
            }

            _SelectedMessageIt--;
        }

        if (inputID == BUTTON_1)
        {
            LoraUtils::DeleteSavedMessage(_SelectedMessageIt);

            if (_SelectedMessageIt == LoraUtils::SavedMessageListEnd() && LoraUtils::GetSavedMessageListSize() > 0)
            {
                _SelectedMessageIt--;
            }
        }
    }

    void displayState()
    {
        Window_State::displayState();

        if (LoraUtils::GetSavedMessageListSize() > 0) 
        {
            auto msgStr = *_SelectedMessageIt;
            Display_Utils::printCenteredText(msgStr.c_str(), true);

            if (_ScrollWheelPatternID > -1)
            {
                StaticJsonDocument<128> doc;
                doc["numItems"] = LoraUtils::GetSavedMessageListSize();
                doc["currItem"] = std::distance(LoraUtils::SavedMessageListBegin(), _SelectedMessageIt);

                LED_Utils::configurePattern(_ScrollWheelPatternID, doc);
                LED_Utils::iteratePattern(_ScrollWheelPatternID);
            }
        }
        else
        {
            Display_Utils::printCenteredText("No Saved Messages", true);
        }
        
    }

protected:

    std::vector<std::string>::iterator _SelectedMessageIt;
    int _ScrollWheelPatternID;
};
