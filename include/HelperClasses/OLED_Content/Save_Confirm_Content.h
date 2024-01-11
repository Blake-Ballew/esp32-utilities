#pragma once

#include "OLED_Content.h"
#include "Settings_Manager.h"
#include "Message_Types.h"

class Save_Confirm_Content : public OLED_Content
{
public:
    Save_Confirm_Content(Adafruit_SSD1306 *disp);

    ~Save_Confirm_Content();

    void printContent();

    void encUp();
    void encDown();

    void saveMsg(Message_Ping *msg);
    void saveCoordinates(Message_Ping *msg);
};