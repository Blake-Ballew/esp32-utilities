#pragma once

#include "OLED_Window.h"
#include "Received_Messages_Content.h"
#include "LoraMessageDisplay.h"
#include "Ping_Content.h"

class Statuses_Window : public OLED_Window
{
public:
    Statuses_Window(OLED_Window *parent);
    ~Statuses_Window();

    void execBtnCallback(uint8_t inputID);
    void Pause()
    {
        isPaused = true;
        LED_Manager::clearRing();
    }

    void Resume()
    {
        isPaused = false;
        content->printContent();
    }

private:
    
};