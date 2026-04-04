// SUPERSEDED — generic OLED_Content_List scrolling state.
// In the new DisplayModule system, use SelectKeyValueState (for key/value
// selection) or build draw commands directly in a WindowState subclass.
// Safe to delete once the legacy Window_States tree is retired.

#pragma once

#include "Window_State.h"
#include "OLED_Content.h"
#include "DisplayTypes.hpp"

class Select_Content_List_State : public Window_State
{
public:
    Select_Content_List_State() {
        Select_Content_List_State(new OLED_Content_List());
    }

    Select_Content_List_State(OLED_Content_List *content)
    {
        
        renderContent = content;
        contentList = content;

        assignInput(DisplayModule::InputID::BUTTON_3, ACTION_BACK, "Back");
        assignInput(DisplayModule::InputID::BUTTON_4, ACTION_SELECT, "Select");
        assignInput(DisplayModule::InputID::ENC_UP, ACTION_DEFER_CALLBACK_TO_WINDOW);
        assignInput(DisplayModule::InputID::ENC_DOWN, ACTION_DEFER_CALLBACK_TO_WINDOW);
    }

    ~Select_Content_List_State()
    {
    }

    void enterState(State_Transfer_Data &transferData)
    {
    }

    void exitState(State_Transfer_Data &transferData)
    {
        
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case DisplayModule::InputID::ENC_UP:
            renderContent->encUp();
            break;
        case DisplayModule::InputID::ENC_DOWN:
            renderContent->encDown();
            break;
        default:
            break;
        }
    }

    protected:
    OLED_Content_List *contentList;

};