#pragma once

#include "Window_State.h"
#include "Saved_Locations_Content.h"

class Saved_Locations_State : public Window_State
{
public:
    Saved_Locations_State(Saved_Locations_Content *locations = nullptr) : locationsContent(locations)
    {
        //typeID = __COUNTER__;
        if (locations == nullptr)
        {
            locationsContent = new Saved_Locations_Content();
            renderContent = locationsContent;
        }
        else
        {
            renderContent = locations;
        }

        assignInput(ACTION_BACK, BUTTON_3);
        if (locationsContent != nullptr)
        {
        }
    }

    ~Saved_Locations_State()
    {
        if (locationsContent != nullptr)
        {
            locationsContent->stop();
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        // Button 1: Tracking State: (userID, messageID)
        // Button 2: Ping State (false): Only when showRead is true
        // Button 3: Ping State (true, userID)
        // ENC_UP: Home State
        /* switch (transferData.inputID)
        {
        case BUTTON_1:
        {
            auto location = locationsContent->getSelectedLocation();
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["location"] = location;
            transferData.serializedData = doc;
        }
        break;
        case BUTTON_2:
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["isDirect"] = false;
            transferData.serializedData = doc;
        }
        break;
        case BUTTON_3:
        {
            auto location = locationsContent->getSelectedLocation();
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["isDirect"] = true;
            (*doc)["location"] = location;
            transferData.serializedData = doc;
        }
        break;
        case ENC_UP:
        {
            transferData.callbackID = ACTION_SWITCH_WINDOW_STATE;
            transferData.inputID = ACTION_BACK;
        }
        break;
        default:
            break;
        } */
    }

private:
    Saved_Locations_Content *locationsContent;
};