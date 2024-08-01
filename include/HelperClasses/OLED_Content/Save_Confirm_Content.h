#pragma once

#include "OLED_Content.h"
#include "Settings_Manager.h"
#include "MessagePing.h"

class Save_Confirm_Content : public OLED_Content
{
public:
    Save_Confirm_Content(Adafruit_SSD1306 *disp);

    ~Save_Confirm_Content();

    void printContent();

    void encUp();
    void encDown();

    void saveMsg(MessagePing *msg);
    void saveCoordinates(MessagePing *msg);
};