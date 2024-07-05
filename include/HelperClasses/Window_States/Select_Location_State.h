#pragma once

#include "Window_State.h"
#include "Saved_Locations_Content.h"

class Select_Location_State : public Window_State
{
public:
    Select_Location_State(Saved_Locations_Content *locations = nullptr)
    {
        // typeID = __COUNTER__;
        if (locations == nullptr)
        {
            locationsContent = new Saved_Locations_Content();
            renderContent = locationsContent;
        }
        else
        {
            locationsContent = locations;
            renderContent = locations;
        }

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
#if DEBUG == 1
        Serial.println("Select_Location_State::enterState");
#endif
        locationsContent->loadLocations();
        Window_State::enterState(transferData);
    }

    void exitState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Select_Location_State::exitState");
#endif
        Window_State::exitState(transferData);
        DynamicJsonDocument *doc = nullptr;

        switch (transferData.inputID)
        {
        case BUTTON_4:
        {
            doc = new DynamicJsonDocument(128);
            auto location = locationsContent->getSelectedLocation();
            if (location != nullptr)
            {
                if (location->idx == -1)
                {
                    (*doc)["isCurrLocation"] = true;
                }
                else
                {
                    (*doc)["isCurrLocation"] = false;
                    (*doc)["idx"] = location->idx;
                    (*doc)["name"] = location->name;
                    (*doc)["lat"] = location->lat;
                    (*doc)["lon"] = location->lon;
                }
            }

            transferData.serializedData = doc;
        }
        break;
        default:
            break;
        }

        LED_Manager::clearRing();
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
        {
            locationsContent->encUp();
        }
        break;
        case ENC_DOWN:
        {
            locationsContent->encDown();
        }
        break;
        default:
            break;
        }
    }

private:
    Saved_Locations_Content *locationsContent;
};