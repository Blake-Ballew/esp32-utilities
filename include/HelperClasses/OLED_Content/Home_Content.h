#pragma once

#include "OLED_Content.h"
#include "NavigationUtils.h"
#include "FilesystemUtils.h"
#include "LoraUtils.h"
#include "System_Utils.h"

class Home_Content : public OLED_Content
{
public:
    const static uint8_t HOME_CONTENT_MAIN = 1;
    const static uint8_t HOME_CONTENT_MESSAGES = 2;
    const int HOME_CONTENT_TIMER_PERIOD = 60000;

    uint8_t contentMode;

    Home_Content(Adafruit_SSD1306 *display);
    ~Home_Content();

    void printContent();
    void encUp();
    void encDown();

    void stop();
    void start();

    MessageBase *getCurrentMessage();

private:
    std::map<uint64_t, MessageBase *>::iterator msgIterator;
};