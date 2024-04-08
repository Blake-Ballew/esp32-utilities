#include "OLED_Window.h"
#include "Navigation_Manager.h"
#include "GPS_Content.h"
#include "Edit_String_Content.h"

class GPS_Window : public OLED_Window
{
public:
    GPS_Window(OLED_Window *parent);
    ~GPS_Window();

    void execBtnCallback(uint8_t inputID);

private:
    GPS_Content *gpsContent;
    Edit_String_Content *editStringContent;
    static GPS_Window *thisInstance;
    static TimerHandle_t updateTimer;
    static StaticTimer_t updateTimerBuffer;
    static void updateGPS(TimerHandle_t xTimer);
    static void handleBtnInterrupt(uint8_t inputID, OLED_Window *window);
};