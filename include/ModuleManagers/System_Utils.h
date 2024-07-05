#pragma once

#include "globalDefines.h"
#include <Arduino.h>
#include <unordered_map>
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "ArduinoJson.h"
#include "ArduinoOTA.h"

namespace 
{
    const char * COMMAND_FIELD PROGMEM = "cmd";
    const char * DISPLAY_BUFFER_FIELD PROGMEM = "buffer";
    const char * DISPLAY_WIDTH PROGMEM = "width";
    const char * DISPLAY_HEIGHT PROGMEM = "height";
}

enum DebugCommand
{
    DISPLAY_CONTENTS = 0,
    REGISTER_INPUT = 1,
};

class System_Utils
{
public:
    static bool silentMode;
    static Adafruit_SSD1306 *OLEDdisplay;

    static void init(Adafruit_SSD1306 *display);
    static long getBatteryPercentage();
    static void monitorSystemHealth(TimerHandle_t xTimer);
    static void shutdownBatteryWarning();

    // Timer functionality
    static int registerTimer(const char *timerName, size_t periodMS, TimerCallbackFunction_t callback);
    static int registerTimer(const char *timerName, size_t periodMS, TimerCallbackFunction_t callback, StaticTimer_t &timerBuffer);

    static void deleteTimer(int timerID);

    static bool isTimerActive(int timerID);
    static void startTimer(int timerID);
    static void stopTimer(int timerID);

    static void resetTimer(int timerID);

    static void changeTimerPeriod(int timerID, size_t timerPeriodMS);

    // OTA Firmware Update

    static bool otaInitialized;
    static void initializeOTA();
    static void stopOTA();

    // Debug Companion

    static void sendDisplayContents(Adafruit_SSD1306 *display);

private:

    static std::unordered_map<int, TimerHandle_t> systemTimers;
    static int nextTimerID;

    static StaticTimer_t healthTimerBuffer;
    static int healthTimerID;
};