#pragma once

#include "OLED_Window.h"
#include "Repeat_Message_Content.h"

class SOS_Window : public OLED_Window
{
public:
    SOS_Window(OLED_Window *parent);
    ~SOS_Window();

    void execBtnCallback(uint8_t inputID);
    void Pause();
    void Resume();

private:
    Repeat_Message_Content *sosContent;
};