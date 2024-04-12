#pragma once

#include "globalDefines.h"
#include <Arduino.h>
#include <unordered_map>
#include "LED_Manager.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
//#include "OLED_Manager.h"

class System_Utils
{
public:
    static bool silentMode;
    static Adafruit_SSD1306 *display;

    static void init(Adafruit_SSD1306 *display);
    static long getBatteryPercentage();
    static void monitorSystemHealth(TimerHandle_t xTimer);
    static void shutdownBatteryWarning();

    // Timer functionality
    static int registerTimer(const char * timerName, size_t periodMS, TimerCallbackFunction_t callback);
    static int registerTimer(const char * timerName, size_t periodMS, TimerCallbackFunction_t callback, StaticTimer_T &timerBuffer);

    static void startTimer(int timerID);
    static void stopTimer(int timerID);

    static void changeTimerPeriod(int timerID, size_t timerPeriodMS);

private:
    static std::unordered_map<int, TimerHandle_t> systemTimers;
    static int nextTimerID;
    static TimerHandle_t healthTimer;
    static StaticTimer_t healthTimerBuffer;
};