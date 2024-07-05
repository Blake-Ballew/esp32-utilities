#pragma once

#include "OLED_Window.h"
#include "OLED_Manager.h"
#include "Settings_Manager.h"
#include "Settings_Content.h"
#include "Edit_States.h"
#include "Settings_State.h"
#include <Arduino.h>
#include <ArduinoJson.h>

class Settings_Window : public OLED_Window
{
public:
    Settings_Window(OLED_Window *parent);
    ~Settings_Window();

    void callFunctionState(uint8_t inputID);
    void returnFromFunctionState(uint8_t inputID);

    void transferState(State_Transfer_Data &transferData);

    // void execBtnCallback(uint8_t inputID);
    // void drawWindow();

    // void execBtnCallback(uint8_t buttonNumber, void *arg);

protected:
    Settings_State *settingsState;
    Edit_Bool_State *editBoolState;
    Edit_Int_State *editIntState;
    Edit_Float_State *editFloatState;
    Edit_String_State *editStringState;
    Edit_Enum_State *editEnumState;

    bool saveSettings;
};
