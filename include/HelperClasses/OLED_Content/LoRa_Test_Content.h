#pragma once

#include "OLED_Content.h"
#include "LoraUtils.h"
#include "Navigation_Manager.h"
#include "Settings_Manager.h"

class LoRa_Test_Content : public OLED_Content
{
public:
    LoRa_Test_Content(Adafruit_SSD1306 *disp);
    ~LoRa_Test_Content();

    void printContent();
    void encUp();
    void encDown();

    uint8_t sendBroadcast();
    void updateMessages();

    MessageBase *getCurrentMessage();

private:
    uint16_t msgIdx = 0;
};