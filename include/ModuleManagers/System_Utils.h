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
#include "EventHandler.h"

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
    // static Adafruit_SSD1306 *OLEDdisplay;

    static void init();
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

    // Debug Companion Functionality
    static void sendDisplayContents(Adafruit_SSD1306 *display);

    // Event Handler public invoke functions
    static void enableInterruptsInvoke();
    static void disableInterruptsInvoke();
    static void systemShutdownInvoke();

    // Event Handler Getters
    static EventHandler &getEnableInterrupts() { return enableInterrupts; }
    static EventHandler &getDisableInterrupts() { return disableInterrupts; }
    static EventHandler &getSystemShutdown() { return systemShutdown; }

private:
    // Event Handlers
    static EventHandler enableInterrupts;
    static EventHandler disableInterrupts;
    static EventHandler systemShutdown;

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