#pragma once

#include "Window_State.h"
#include "NavigationUtils.h"
#include "ScrollWheel.h"
#include "LED_Utils.h"
#include "MessagePing.h"

// Displays a list of saved locations
// Inputs:
//   ENC_UP: Select next location
//   ENC_DOWN: Select previous location
//   BUTTON_3: Return
//   BUTTON_4: Edit
//   BUTTON_1: Delete
//   BUTTON_2: Create new saved location
class Saved_Locations_State : public Window_State
{
public:
    Saved_Locations_State()
    {
        assignInput(ENC_UP, ACTION_DEFER_CALLBACK_TO_WINDOW);
        assignInput(ENC_DOWN, ACTION_DEFER_CALLBACK_TO_WINDOW);

        assignInput(BUTTON_3, ACTION_BACK, "Back");
        assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Edit");
        assignInput(BUTTON_1, ACTION_DEFER_CALLBACK_TO_WINDOW, "Delete");
        assignInput(BUTTON_2, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Track");
    }

    ~Saved_Locations_State()
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
            SavedLocation location = *_SelectedLocationIt;
            location.Name = newStr;

            NavigationUtils::UpdateSavedLocation(_SelectedLocationIt, location);
        }
        else
        {
            _SelectedLocationIt = NavigationUtils::GetSavedLocationsBegin();
        }

        if (NavigationUtils::GetSavedLocationsSize() == 0)
        {
            buttonCallbacks.erase(BUTTON_2);
        }
        else
        {
            assignInput(BUTTON_2, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Track");
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        LED_Utils::disablePattern(_ScrollWheelPatternID);

        if (transferData.inputID == BUTTON_4)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(128);

            (*doc)["cfgVal"] = _SelectedLocationIt->Name;
            (*doc)["maxLen"] = STATUS_LENGTH;

            transferData.serializedData = doc;
        }
        // Output location coordinates and name
        else if (transferData.inputID == BUTTON_2)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(256);
            
            (*doc)["lat"] = _SelectedLocationIt->Latitude;
            (*doc)["lon"] = _SelectedLocationIt->Longitude;

            (*doc)["color_R"] = LED_Utils::ThemeColor().r;
            (*doc)["color_G"] = LED_Utils::ThemeColor().g;
            (*doc)["color_B"] = LED_Utils::ThemeColor().b;

            auto displayArr = (*doc).createNestedArray("displayTxt");

            displayArr.add(_SelectedLocationIt->Name);

            transferData.serializedData = doc;
        }
    }

    void processInput(uint8_t inputID)
    {
        if (inputID == ENC_DOWN)
        {
            _SelectedLocationIt++;

            if (_SelectedLocationIt == NavigationUtils::GetSavedLocationsEnd())
            {
                _SelectedLocationIt = NavigationUtils::GetSavedLocationsBegin();
            }
        }

        if (inputID == ENC_UP)
        {
            if (_SelectedLocationIt == NavigationUtils::GetSavedLocationsBegin())
            {
                _SelectedLocationIt = NavigationUtils::GetSavedLocationsEnd();
            }

            _SelectedLocationIt--;
        }

        if (inputID == BUTTON_1)
        {
            NavigationUtils::RemoveSavedLocation(_SelectedLocationIt);

            if (_SelectedLocationIt == NavigationUtils::GetSavedLocationsEnd() && NavigationUtils::GetSavedLocationsSize() > 0)
            {
                _SelectedLocationIt--;
            }
        }
    }

    void displayState()
    {
        Window_State::displayState();

        if (NavigationUtils::GetSavedLocationsSize() > 0) 
        {
            auto location = *_SelectedLocationIt;
            Display_Utils::printCenteredText(location.Name.c_str(), true);

            if (_ScrollWheelPatternID > -1)
            {
                StaticJsonDocument<128> doc;
                doc["numItems"] = NavigationUtils::GetSavedLocationsSize();
                doc["currItem"] = std::distance(NavigationUtils::GetSavedLocationsBegin(), _SelectedLocationIt);

                LED_Utils::configurePattern(_ScrollWheelPatternID, doc);
                LED_Utils::iteratePattern(_ScrollWheelPatternID);
            }
        } 
        else
        {
            Display_Utils::printCenteredText("No Saved Locations", true);
        }
        
    }

protected:
    std::vector<SavedLocation>::iterator _SelectedLocationIt;
    int _ScrollWheelPatternID = -1;
};