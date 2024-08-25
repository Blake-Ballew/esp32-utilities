#pragma once

#include "NavigationUtils.h"
#include "OLED_Content.h"

class GPS_Content : public OLED_Content
{
public:
    GPS_Content();
    ~GPS_Content();

    void printContent();
    void encUp();
    void encDown();

private:
};