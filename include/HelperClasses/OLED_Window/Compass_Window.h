#pragma once

#include "OLED_Window.h"
#include "QMC5883LCompass.h"
#include "Compass_Content.h"
#include "Compass_States.h"
#include "LED_Manager.h"

class Compass_Window : public OLED_Window
{
public:
    Compass_Window(OLED_Window *parent);
    ~Compass_Window();

    void execBtnCallback(uint8_t inputID) {}

    void Pause();
    void Resume();

private:
    Compass_Content *compassContent;

    Compass_Display_State *compassState;
};