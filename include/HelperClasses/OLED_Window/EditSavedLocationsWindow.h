#pragma once

#include "OLED_Window.h"
#include "Saved_Locations_State.h"
#include "SaveLocationState.h"
#include "Edit_States.h"

class EditSavedLocationsWindow : public OLED_Window
{
public:
    EditSavedLocationsWindow(OLED_Window *parent) : OLED_Window(parent)
    {
        Edit_String_Content *editStringContent = new Edit_String_Content();
        contentList.push_back(editStringContent);

        savedLocationsState = new Saved_Locations_State();
        stateList.push_back(savedLocationsState);

        editStringState = new Edit_String_State(editStringContent);
        stateList.push_back(editStringState);

        saveLocationState = new SaveLocationState(editStringContent);
        stateList.push_back(saveLocationState);

        setInitialState(savedLocationsState);

        savedLocationsState->setAdjacentState(BUTTON_4, editStringState);
        savedLocationsState->setAdjacentState(BUTTON_2, saveLocationState);
    }

    ~EditSavedLocationsWindow() {}

protected:
    Saved_Locations_State *savedLocationsState;
    Edit_String_State *editStringState;
    SaveLocationState *saveLocationState;
};