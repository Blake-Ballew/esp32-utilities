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

#ifdef USE_BLE
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#endif

namespace 
{
    const char * COMMAND_FIELD PROGMEM = "cmd";
    const char * DISPLAY_BUFFER_FIELD PROGMEM = "buffer";
    const char * DISPLAY_WIDTH PROGMEM = "width";
    const char * DISPLAY_HEIGHT PROGMEM = "height";

    const char *DEVICE_NAME PROGMEM = "ESP32";

    #ifdef USE_BLE
    const char *BLE_SERVICE_UUID PROGMEM = "5d49ff2d-b739-4d50-945f-1c5741ca6e2f";
    #endif
    // BLECharacteristic BLE_CHARACTERISTIC_SEND_MSGPACK_UUID("08ba298d-31f2-4f68-b24a-15d23c4cfbd3", BLECharacteristic::PROPERTY_NOTIFY);
    // BLEDescriptor BLE_DESCRIPTOR_SEND_MSGPACK_UUID(BLEUUID((uint16_t)0x2902));

    // BLECharacteristic BLE_CHARACTERISTIC_RECEIVE_MSGPACK_UUID("8dd684a5-0090-4c81-9793-ac7f0d0d6e20", BLECharacteristic::PROPERTY_WRITE);
    // BLEDescriptor BLE_DESCRIPTOR_RECEIVE_MSGPACK_UUID(BLEUUID((uint16_t)0x2903));
}

const uint8_t ADC_WIFI = 0;
const uint8_t ADC_BT = 1;

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

    // Queue functionality
    static int registerQueue(size_t queueLength, size_t itemSize);
    static int registerQueue(size_t queueLength, size_t itemSize, uint8_t *queueBuffer, StaticQueue_t &queueBuffer);
    static QueueHandle_t getQueue(int queueID);
    static void deleteQueue(int queueID);
    static void resetQueue(int queueID);
    static bool sendToQueue(int queueID, void *item, size_t timeoutMS); 

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

    // Bluetooth functionality
    #ifdef USE_BLE
    static void initBluetooth();
    static void gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    static void gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    // static void enableBluetooth();
    // static void disableBluetooth();
    // static void addCharacteristic(BLECharacteristic &characteristic);
    #endif

    // WiFi functionality
    static bool enableWiFi();
    static void disableWiFi();
    static IPAddress getLocalIP();

    // 2.4Ghz Radio functionality
    static void enableRadio(uint8_t adcUser);
    static void disableRadio(uint8_t adcUser);

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
    static EventHandlerT<uint8_t> &getInputRaised() { return inputRaised; }

private:
    // Event Handlers
    static EventHandler enableInterrupts;
    static EventHandler disableInterrupts;
    static EventHandler systemShutdown;
    static EventHandlerT<uint8_t> inputRaised;

    // Timer functionality
    static std::unordered_map<int, TimerHandle_t> systemTimers;
    static int nextTimerID;

    // Task functionality
    static std::unordered_map<int, TaskHandle_t> systemTasks;
    static int nextTaskID;

    // Queue functionality
    static std::unordered_map<int, QueueHandle_t> systemQueues;
    static int nextQueueID;

    // ADC Users
    static std::unordered_map<uint8_t, bool> adcUsers;

    static StaticTimer_t healthTimerBuffer;
    static int healthTimerID;
    static int otaTaskID;
};