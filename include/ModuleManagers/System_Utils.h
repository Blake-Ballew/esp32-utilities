#pragma once

#include "globalDefines.h"
#include <Arduino.h>
#include <unordered_map>
#include <vector>
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "ArduinoJson.h"
#include "ArduinoOTA.h"
#include "driver/adc.h"

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

struct InterruptCallbackSet
{
    void (*enableInterrupt)();
    void (*disableInterrupt)();
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

    // Interrupt functionality
    // Register interrupt enable/disable functions with the system so they can be disabled when needed
    static void registerInterrupt(void (*enableInterrupt)(), void (*disableInterrupt)());
    static void enableInterrupts();
    static void disableInterrupts();


    // Timer functionality
    static int registerTimer(const char *timerName, size_t periodMS, TimerCallbackFunction_t callback);
    static int registerTimer(const char *timerName, size_t periodMS, TimerCallbackFunction_t callback, StaticTimer_t &timerBuffer);
    static void deleteTimer(int timerID);
    static bool isTimerActive(int timerID);
    static void startTimer(int timerID);
    static void stopTimer(int timerID);
    static void resetTimer(int timerID);
    static void changeTimerPeriod(int timerID, size_t timerPeriodMS);


    // Task functionality
    // Dynamic memory allocation, no pinned core
    static int registerTask(
        TaskFunction_t taskFunction, 
        const char *taskName, 
        uint32_t taskStackSize, 
        void *taskParameters, 
        UBaseType_t taskPriority);

    // Dynamic memory allocation, pinned to core
    static int registerTask(
        TaskFunction_t taskFunction, 
        const char *taskName, 
        uint32_t taskStackSize, 
        void *taskParameters, 
        UBaseType_t taskPriority,
         BaseType_t coreID);

    // Static memory allocation, no pinned core
    static int registerTask(
        TaskFunction_t taskFunction, 
        const char *taskName, 
        uint32_t taskStackSize, 
        void *taskParameters, 
        UBaseType_t taskPriority, 
        StackType_t &stackBuffer, 
        StaticTask_t &taskBuffer);

    // Static memory allocation, pinned to core
    static int registerTask(
        TaskFunction_t taskFunction,
        const char *taskName, 
        uint32_t taskStackSize, 
        void *taskParameters, 
        UBaseType_t taskPriority, 
        StackType_t &stackBuffer, 
        StaticTask_t &taskBuffer, 
        BaseType_t coreID);    

    static void suspendTask(int taskID);
    static void resumeTask(int taskID);
    static void deleteTask(int taskID);

    // WiFi functionality
    static bool enableWiFi();
    static void disableWiFi();
    static IPAddress getLocalIP();

    // OTA Firmware Update

    static bool otaInitialized;
    static int otaTimerID;
    static bool initializeOTA();
    static void startOTA();
    static void stopOTA();

    // Debug Companion

    static void sendDisplayContents(Adafruit_SSD1306 *display);

private:
    // Interrupt functionality
    static std::vector<InterruptCallbackSet> interruptCallbacks;

    // Timer functionality
    static std::unordered_map<int, TimerHandle_t> systemTimers;
    static int nextTimerID;

    // Task functionality
    static std::unordered_map<int, TaskHandle_t> systemTasks;
    static int nextTaskID;

    static StaticTimer_t healthTimerBuffer;
    static int healthTimerID;
    static int otaTaskID;
};