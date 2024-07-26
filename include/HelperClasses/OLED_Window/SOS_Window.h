#pragma once

#include "OLED_Window.h"
#include "Repeat_Message_Content.h"
#include "Repeat_Message_State.h"
#include "Confirm_State.h"

class SOS_Window : public OLED_Window
{
public:
    SOS_Window(OLED_Window *parent, 
    Repeat_Message_State *repeat,
    Confirm_State *confirm);

    ~SOS_Window();

    void execBtnCallback(uint8_t inputID);
    void Pause();
    void Resume();

private:
    // Repeat_Message_Content *sosContent;
    Repeat_Message_State *sosState;
    Confirm_State *confirmState;
};