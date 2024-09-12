#pragma once

#include "Window_State.h"
#include "Home_Content.h"

class Home_State : public Window_State
{
public:
    Home_State(Home_Content *home) : homeContent(home)
    {
        //typeID = __COUNTER__;
        renderContent = home;

        assignInput(ENC_DOWN, ACTION_SWITCH_WINDOW_STATE);
        assignInput(BUTTON_1, ACTION_GENERATE_QUICK_ACTION_MENU, "Actions");
        assignInput(BUTTON_2, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Broadcast");
        assignInput(BUTTON_3, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Lock");
        assignInput(BUTTON_4, ACTION_GENERATE_MENU_WINDOW, "Main Menu");
    }

    ~Home_State()
    {
    }

    void enterState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Home_State::enterState");
#endif
        Window_State::enterState(transferData);
    }

    void exitState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Home_State::exitState");
#endif

        Window_State::exitState(transferData);

        if (transferData.inputID == BUTTON_2)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["sendDirect"] = false;
            transferData.serializedData = doc;
        }
    }

    void processInput(uint8_t inputID)
    {
        uint8_t contentMode = homeContent->contentMode;
        switch (inputID)
        {
        case BUTTON_1:
            break;
        case BUTTON_2:
            break;
        case BUTTON_3:
            break;
        case BUTTON_4:
            break;
        case ENC_DOWN:
            break;
        case ENC_UP:
            break;
        default:
            break;
        }
    }

private:
    Home_Content *homeContent;
};
