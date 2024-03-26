#pragma once

#include "OLED_Content.h"

enum Confirm_State
{
    CONFIRM_STATE_YES,
    CONFIRM_STATE_NO,
    CONFIRM_STATE_UNSET
};

class Confirm_Content : public OLED_Content
{
public:
    Confirm_Content(Adafruit_SSD1306 *disp);

    ~Confirm_Content();

    void printContent();

    void encUp() {}
    void encDown() {}

    void passButtonPress(uint8_t inputID) {}

    Confirm_State state = CONFIRM_STATE_UNSET;
};