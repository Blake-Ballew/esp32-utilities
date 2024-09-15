#include "GPS_Window.h"

GPS_Window::GPS_Window(OLED_Window *parent) : OLED_Window(parent)
{
    assignButton(ACTION_BACK, BUTTON_3, "Back", 4);
}

GPS_Window::~GPS_Window()
{

}


