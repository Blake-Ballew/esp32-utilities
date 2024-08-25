#pragma once

#include "Window_State.h"
// #include "Saved_Locations_Content.h"

namespace
{
    const char *LOC_SELECT PROGMEM = "Select a location";
    const char *NO_LOC PROGMEM = "No locations";
}

class Select_Location_State : public Window_State
{
public:
    Select_Location_State()
    {
        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Select");
        assignInput(ENC_UP, ACTION_DEFER_CALLBACK_TO_WINDOW);
        assignInput(ENC_DOWN, ACTION_DEFER_CALLBACK_TO_WINDOW);
    }

    ~Select_Location_State()
    {
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        locations.clear();

        if (transferData.serializedData != nullptr)
        {
            DynamicJsonDocument *doc = transferData.serializedData;
            if (doc->containsKey("locations") && (*doc)["locations"].is<JsonArray>())
            {
                for (auto loc : (*doc)["locations"].as<JsonArray>())
                {
                    SavedLocation location;
                    location.Name = loc["name"].as<std::string>();
                    location.Latitude = loc["lat"].as<double>();
                    location.Longitude = loc["lng"].as<double>();

                    locations.push_back(location);
                }

                locationIt = locations.begin();
            }
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);
        DynamicJsonDocument *doc = nullptr;

        if (locationIt != locations.end() && transferData.inputID == BUTTON_4)
        {
            doc = new DynamicJsonDocument(128);
            auto location = *locationIt;

            (*doc)["name"] = location.Name;
            (*doc)["lat"] = location.Latitude;
            (*doc)["lng"] = location.Longitude;

            transferData.serializedData = doc;
        }

        LED_Manager::clearRing();
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
        {
            if (locations.size() > 1)
            {
                if (locationIt == locations.begin())
                {
                    locationIt = locations.end() - 1;
                }
                else
                {
                    locationIt--;
                }
            }
        }
        break;
        case ENC_DOWN:
        {
            if (locations.size() > 1)
            {
                locationIt++;

                if (locationIt == locations.end())
                {
                    locationIt = locations.begin();
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
        Display_Utils::clearContentArea();

        if (locations.size() > 0)
        {
            SavedLocation location = *locationIt;
            
            TextFormat prompt;
            prompt.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            prompt.verticalAlignment = TEXT_LINE;
            prompt.line = 2;

            Display_Utils::printFormattedText(LOC_SELECT, prompt);

            TextFormat locationText;
            locationText.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            locationText.verticalAlignment = TEXT_LINE;
            locationText.line = 3;

            Display_Utils::printFormattedText(location.Name.c_str(), locationText);
        }
        else
        {
            TextFormat prompt;
            prompt.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            prompt.verticalAlignment = ALIGN_CENTER_VERTICAL;

            Display_Utils::printFormattedText(NO_LOC, prompt);
        }

        display->display();
    }

private:
    std::vector<SavedLocation> locations;
    std::vector<SavedLocation>::iterator locationIt;
};