#pragma once

#include "OLED_Content.h"
#include "Settings_Manager.h"
#include "LED_Manager.h"

class Saved_Msg_Content : public OLED_Content
{
public:
    Saved_Msg_Content(Adafruit_SSD1306 *disp);

    ~Saved_Msg_Content();

    void printContent();

    void encUp();
    void encDown();

    void deleteMsg();
    void saveMsg(const char *msg, uint8_t msgLength);

private:
    size_t msgIdx;
    size_t msgListSize;
};