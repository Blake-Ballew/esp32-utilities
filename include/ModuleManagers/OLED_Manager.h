#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <map>
#include <unordered_map>
#include <vector>
#include "EventDeclarations.h"
#include "OLED_Window.h"
#include "OLED_Settings_Window.h"
#include "Compass_Window.h"
#include "GPS_Window.h"
#include "LoRa_Test_Window.h"
#include "Statuses_Window.h"
#include "Ping_Window.h"
#include "Home_Window.h"
#include "SOS_Window.h"
#include "Saved_Msg_Window.h"
#include "globalDefines.h"
// #include "esp_event_base.h"

#define DEBOUNCE_DELAY 100

using callbackPointer = void (*)(uint8_t);
using inputCallbackPointer = void (*)();

class OLED_Manager
{
public:
    static OLED_Manager *instance;

    static Adafruit_SSD1306 display;

    static OLED_Window *currentWindow;
    static OLED_Window *rootWindow;

    static std::map<uint32_t, callbackPointer> callbackMap;
    static std::map<uint8_t, inputCallbackPointer> inputCallbackMap;
    static std::unordered_map<size_t, uint8_t> inputMap;

    static void init();

    static OLED_Window *attachNewWindow();
    static void attachNewWindow(OLED_Window *window);

    // Input handler using FreeRTOS task notifications.
    static void processButtonPressEvent(void *taskParams);
    static void initializeCallbacks();
    static void processEventCallback(uint32_t resourceID, uint8_t inputID);
    static void processInputCallback(uint8_t inputID);
    static void registerCallback(uint32_t resourceID, callbackPointer callback);
    static void registerInputCallback(uint8_t inputID, inputCallbackPointer callback);
    static void registerInput(uint32_t resourceID, uint8_t inputID);
    static void displayLowBatteryShutdownNotice();

    // Callbacks
    static void goBack(uint8_t inputID);
    static void select(uint8_t inputID);
    static void generateHomeWindow(uint8_t inputID);
    static void generateSettingsWindow(uint8_t inputID);
    static void generatePingWindow(uint8_t inputID);
    static void generateStatusesWindow(uint8_t inputID);
    static void generateMenuWindow(uint8_t inputID);
    static void generateCompassWindow(uint8_t inputID);
    static void generateGPSWindow(uint8_t inputID);
    static void generateLoRaTestWindow(uint8_t inputID);
    static void flashDefaultSettings(uint8_t inputID);
    static void rebootDevice(uint8_t inputID);
    static void toggleFlashlight(uint8_t inputID);
    static void shutdownDevice(uint8_t inputID);
    static void toggleSilentMode(uint8_t inputID);
    static void quickActionMenu(uint8_t inputID);
    static void openSOS(uint8_t inputID);
    static void openSavedMsg(uint8_t inputID);
    static void switchWindowState(uint8_t inputID);
    static void callFunctionalWindowState(uint8_t inputID);
    static void returnFromFunctionWindowState(uint8_t inputID);
    static void callFunctionWindowState(uint8_t inputID);

    // Input callbacks
    static void processMessageReceived();

private:
    static TickType_t lastButtonPressTick;
    static std::vector<uint8_t> getInputsFromNotification(uint32_t notification);
};