#pragma once

#include "OLED_Window.h"
#include "Edit_States.h"
#include "SavedStatusMsgListState.h"
#include "SaveStatusMessageState.h"

class EditStatusMessagesWindow : public OLED_Window
{
public:
    EditStatusMessagesWindow(OLED_Window *parent) : OLED_Window(parent)
    {
        Edit_String_Content *editStringContent = new Edit_String_Content();

        savedStatusMsgListState = new SavedStatusMsgListState();
        stateList.push_back(savedStatusMsgListState);

        editStringState = new Edit_String_State(editStringContent);
        stateList.push_back(editStringState);

        saveStatusMessageState = new SaveStatusMessageState(editStringContent);
        stateList.push_back(saveStatusMessageState);

        contentList.push_back(editStringContent);

        setInitialState(savedStatusMsgListState);

        savedStatusMsgListState->setAdjacentState(BUTTON_4, editStringState);
        savedStatusMsgListState->setAdjacentState(BUTTON_2, saveStatusMessageState);
    }

    ~EditStatusMessagesWindow() {}

private:
    SavedStatusMsgListState *savedStatusMsgListState;
    Edit_String_State *editStringState;
    SaveStatusMessageState *saveStatusMessageState;
};