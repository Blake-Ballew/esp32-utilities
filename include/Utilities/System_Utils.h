#pragma once

#include <Arduino.h>
#include <unordered_map>
#include <vector>
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "ArduinoJson.h"
#include "ArduinoOTA.h"
#include "driver/adc.h"
#include "EventHandler.h"
#include <string>
#include <map>
#include <ezTime.h>
#include "TimeSourceInterface.hpp"

#include "esp_ota_ops.h"
#include "mbedtls/base64.h"

#include "esp_rom_crc.h"
#include "esp_log.h"

#include "FilesystemUtils.h"
#include "Bluetooth_Utils.h"
#include "VersionUtils.h"
#include "SettingsInterface.hpp"

static const char *TAG = "System_Utils";

namespace 
{
    const char * COMMAND_FIELD PROGMEM = "cmd";
    const char * DISPLAY_BUFFER_FIELD PROGMEM = "buffer";
    const char * DISPLAY_WIDTH PROGMEM = "width";
    const char * DISPLAY_HEIGHT PROGMEM = "height";

    const char *DEVICE_NAME PROGMEM = "ESP32";
}

struct {
    esp_ota_handle_t handle = 0;
    const esp_partition_t* partition = nullptr;
    size_t total_size = 0;
    size_t bytes_written = 0;
    bool active = false;
} ota_state;

enum DebugCommand
{
    DISPLAY_CONTENTS = 0,
    REGISTER_INPUT = 1,
};

class System_Utils
{
public:
    static std::string DeviceName;
    static size_t DeviceID;

    static bool silentMode;
    static bool time24Hour;
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
    static int registerQueue(size_t queueLength, size_t itemSize, uint8_t *queueData, StaticQueue_t &queueBuffer);
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
    static TaskHandle_t getTask(int taskID);

    // Bluetooth functionality
    static void initBluetooth();

    // WiFi functionality
    static bool enableWiFi();
    static void disableWiFi();
    static IPAddress getLocalIP();

    // 2.4Ghz Radio functionality
    static void enableRadio();
    static void disableRadio();

    // OTA Firmware Update
    static bool otaInitialized;
    static int otaTimerID;
    static bool initializeOTA();
    static void startOTA();
    static void stopOTA();

    // RPC OTA
    static int DecodeBase64(const char* input, uint8_t* output, size_t output_len);
    static void StartOtaRpc(JsonDocument &doc);
    static void UploadOtaChunkRpc(JsonDocument &doc);
    static void EndOtaRpc(JsonDocument &doc);

    // Debug Companion Functionality
    static void GetSystemInfoRpc(JsonDocument &doc);
    static void sendDisplayContents(Adafruit_SSD1306 *display);

    // Event Handler public invoke functions
    static void enableInterruptsInvoke();
    static void disableInterruptsInvoke();
    static void systemShutdownInvoke();

    // Event Handler Getters
    static EventHandler<> &getEnableInterrupts() { return enableInterrupts; }
    static EventHandler<> &getDisableInterrupts() { return disableInterrupts; }
    static EventHandler<> &getSystemShutdown() { return systemShutdown; }

    static void UpdateSettings(JsonDocument &settings)
    {
        if (settings.containsKey("UserID"))
        {
            DeviceID = 0 | settings["UserID"].as<int>();
        }

        if (settings.containsKey("Device Name"))
        {
            DeviceName = settings["Device Name"].as<std::string>();
        }

        if (settings.containsKey("Silent Mode"))
        {
            silentMode = settings["Silent Mode"].as<bool>();
            ESP_LOGI(TAG, "Assigning silentMode %d", silentMode);
        }

        if (settings.containsKey("24H Time"))
        {
            time24Hour = settings["24H Time"].as<bool>();
        }

        if (settings.containsKey("Timezone"))
        {
            const char* posix = _PosixForTimezone(settings["Timezone"].as<int>());
            LocalTimezone().setPosix(posix);
            ESP_LOGI(TAG, "Timezone set to %s", posix);
        }
    }

    static void GenerateDefaultSettings(std::map<std::string, std::shared_ptr<FilesystemModule::SettingsInterface>> &settings)
    {
        auto silentMode = std::make_shared<FilesystemModule::BoolSetting>("Silent Mode", false);
        settings[silentMode->key] = silentMode;

        auto time24hr = std::make_shared<FilesystemModule::BoolSetting>("24H Time", false);
        settings[time24hr->key] = time24hr;

        auto timezone = std::make_shared<FilesystemModule::EnumSetting>(
            "Timezone",
            1,
            std::vector<std::string>{
                "UTC",
                "US/Eastern",
                "US/Central",
                "US/Mountain",
                "US/Pacific",
                "US/Alaska",
                "US/Hawaii",
                "Europe/London",
                "Europe/Central",
                "Europe/Eastern",
                "Asia/Dubai",
                "Asia/Kolkata",
                "Asia/Bangkok",
                "Asia/Shanghai",
                "Asia/Tokyo",
                "Australia/Sydney",
            },
            std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
        );
        settings[timezone->key] = timezone;
    }

    // Time Management
    static void RegisterTimeSource(SystemModule::TimeSourceInterface* source)
    {
        TimeSources().push_back(source);
    }

    static bool GetCurrentUTC(time_t& outTime)
    {
        for (auto* src : TimeSources()) {
            if (src->TryGetCurrentUTC(outTime)) {
                UTC.setTime(outTime);
                return true;
            }
        }
        return false;
    }

    static time_t GetCurrentLocal()
    {
        return LocalTimezone().now();
    }

    static bool IsTimeValid()
    {
        time_t dummy = 0;
        return GetCurrentUTC(dummy);
    }

    static std::string FormatTime(time_t t)
    {
        return std::string(LocalTimezone().dateTime(t, time24Hour ? "H:i" : "g:i A").c_str());
    }

    static std::string FormatDate(time_t t)
    {
        return std::string(LocalTimezone().dateTime(t, "n/j/Y").c_str());
    }

    static std::vector<SystemModule::TimeSourceInterface*>& TimeSources()
    {
        static std::vector<SystemModule::TimeSourceInterface*> sources;
        return sources;
    }

    static Timezone& LocalTimezone()
    {
        static Timezone tz;
        return tz;
    }

    // static int& UTCOffset()
    // {
    //     static int offset = 0;
    //     return offset;
    // }

    // DEBUGGING FUNCTIONS
    static void PrintHeapFragmentation() {
        float frag = static_cast<float>(ESP.getMaxAllocHeap())
            / static_cast<float>(ESP.getFreeHeap());
        ESP_LOGI(TAG, "Heap fragmentation: %.2f%%", frag * 100);
    }

private:
    static const char* _PosixForTimezone(int id)
    {
        static const std::unordered_map<int, const char*> timezones =
        {
            { 0,  "UTC0"                               },  // UTC
            { 1,  "EST5EDT,M3.2.0,M11.1.0"            },  // US/Eastern
            { 2,  "CST6CDT,M3.2.0,M11.1.0"            },  // US/Central
            { 3,  "MST7MDT,M3.2.0,M11.1.0"            },  // US/Mountain
            { 4,  "PST8PDT,M3.2.0,M11.1.0"            },  // US/Pacific
            { 5,  "AKST9AKDT,M3.2.0,M11.1.0"          },  // US/Alaska
            { 6,  "HST10"                              },  // US/Hawaii
            { 7,  "GMT0BST,M3.5.0/1,M10.5.0"          },  // Europe/London
            { 8,  "CET-1CEST,M3.5.0,M10.5.0/3"        },  // Europe/Central
            { 9,  "EET-2EEST,M3.5.0/3,M10.5.0/4"      },  // Europe/Eastern
            { 10, "GST-4"                              },  // Asia/Dubai
            { 11, "IST-5:30"                           },  // Asia/Kolkata
            { 12, "ICT-7"                              },  // Asia/Bangkok
            { 13, "CST-8"                              },  // Asia/Shanghai
            { 14, "JST-9"                              },  // Asia/Tokyo
            { 15, "AEST-10AEDT,M10.1.0,M4.1.0/3"      },  // Australia/Sydney
        };

        auto it = timezones.find(id);
        return it != timezones.end() ? it->second : "UTC0";
    }

    // Event Handlers
    static EventHandler<> enableInterrupts;
    static EventHandler<> disableInterrupts;
    static EventHandler<> systemShutdown;

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