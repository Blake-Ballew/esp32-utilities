#pragma once

#include "Edit_States.h"
#include "NavigationUtils.h"
#include "MessagePing.h"

class SaveLocationState : public Edit_String_State
{
public:
    SaveLocationState() : Edit_String_State()
    {

    }

    SaveLocationState(Edit_String_Content *stringContent) : Edit_String_State(stringContent)
    {

    }

    void enterState(State_Transfer_Data &transferData)
    {
        #if DEBUG == 1
        Serial.println("SaveLocationState::enterState");
        #endif

        if (transferData.serializedData == nullptr)
        {
            lat = NavigationUtils::GetLocation().lat();
            lon = NavigationUtils::GetLocation().lng();

            transferData.serializedData = new DynamicJsonDocument(64);

            (*transferData.serializedData)["maxLen"] = STATUS_LENGTH;
        }

        Edit_String_State::enterState(transferData);

        if (transferData.serializedData != nullptr
        && transferData.serializedData->containsKey("lat")
        && transferData.serializedData->containsKey("lon"))
        {
            lat = (*transferData.serializedData)["lat"].as<double>();
            lon = (*transferData.serializedData)["lon"].as<double>();
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Edit_String_State::exitState(transferData);

        // Only save location if the user confirmed a string
        if (transferData.serializedData != nullptr)
        {
            auto locationStr = (*transferData.serializedData)["return"].as<std::string>();
            SavedLocation location = { locationStr, lat, lon };

            NavigationUtils::AddSavedLocation(location);

            delete transferData.serializedData;
            transferData.serializedData = nullptr;
            
            display->clearDisplay();
            Display_Utils::printCenteredText("Location Saved");
            display->display();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }

protected:
    
    double lat;
    double lon;
};