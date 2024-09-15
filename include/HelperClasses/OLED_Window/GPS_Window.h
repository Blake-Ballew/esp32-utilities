#include "OLED_Window.h"
#include "NavigationUtils.h"
#include "GPS_Content.h"
#include "Edit_String_Content.h"

class GPS_Window : public OLED_Window
{
public:
    GPS_Window(OLED_Window *parent);
    ~GPS_Window();

    // void execBtnCallback(uint8_t inputID);

private:
};