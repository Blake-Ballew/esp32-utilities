#include "System_Utils.h"

bool System_Utils::silentMode = true;

std::unordered_map<int, TimerHandle_t> System_Utils::systemTimers;
int System_Utils::nextTimerID = 0;

std::unordered_map<int, TaskHandle_t> System_Utils::systemTasks;
int System_Utils::nextTaskID = 0;

std::unordered_map<int, QueueHandle_t> System_Utils::systemQueues;
int System_Utils::nextQueueID = 0;

std::unordered_map<uint8_t, bool> System_Utils::adcUsers;

StaticTimer_t System_Utils::healthTimerBuffer;
int System_Utils::healthTimerID;
// Adafruit_SSD1306 *System_Utils::OLEDdisplay = nullptr;
bool System_Utils::otaInitialized = false;
int System_Utils::otaTaskID = -1;

// Event Handlers
EventHandler System_Utils::enableInterrupts;
EventHandler System_Utils::disableInterrupts;
EventHandler System_Utils::systemShutdown;
EventHandlerT<uint8_t> System_Utils::inputRaised;

void System_Utils::init()
{
    healthTimerID = registerTimer("System Health Monitor", 60000, monitorSystemHealth, healthTimerBuffer);
    startTimer(healthTimerID);
    monitorSystemHealth(nullptr);
}

// TODO: Make actual battery curve
long System_Utils::getBatteryPercentage()
{
    uint16_t voltage = analogRead(BATT_SENSE_PIN);

#if DEBUG == 1
    Serial.print("Battery voltage: ");
    Serial.println(voltage);
#endif

    // Show full battery if BATT_SENSE_PIN is low. Device is plugged in.

    if (voltage == 0)
    {
        return 100;
    }

    if (voltage > 2100)
    {
        voltage = 2100;
    }
    else if (voltage < 1750)
    {
        voltage = 1750;
    }
    long percentage = map(voltage, 1750, 2100, 0, 100);
    return percentage;
}

void System_Utils::monitorSystemHealth(TimerHandle_t xTimer)
{
    uint16_t voltage = analogRead(BATT_SENSE_PIN);

    if (voltage < 1750)
    {
        // Battery is low. Shut down.

        // Show message and flash leds before turning off

        digitalWrite(KEEP_ALIVE_PIN, LOW);
    }
}

void System_Utils::shutdownBatteryWarning()
{
    systemShutdown.Invoke();
}


void System_Utils::enableInterruptsInvoke()
{
    #if DEBUG == 1
    Serial.println("Enabling interrupts");
    #endif
    enableInterrupts.Invoke();
}

void System_Utils::disableInterruptsInvoke()
{
    #if DEBUG == 1
    Serial.println("Disabling interrupts");
    #endif
    disableInterrupts.Invoke();
}

void System_Utils::systemShutdownInvoke()
{
    #if DEBUG == 1
    Serial.println("Shutting down system");
    #endif
    systemShutdown.Invoke();
}

int System_Utils::registerTimer(const char *timerName, size_t periodMS, TimerCallbackFunction_t callback)
{
#if DEBUG == 1
    Serial.print("Registering timer: ");
    Serial.println(timerName);
#endif

    TimerHandle_t handle = xTimerCreate(timerName, periodMS, pdTRUE, (void *)0, callback);

    if (handle != nullptr)
    {
        systemTimers[nextTimerID] = handle;
        return nextTimerID++;
    }
    else
    {
        return -1;
    }
}

int System_Utils::registerTimer(const char *timerName, size_t periodMS, TimerCallbackFunction_t callback, StaticTimer_t &timerBuffer)
{
#if DEBUG == 1
    Serial.print("Registering static timer: ");
    Serial.println(timerName);
#endif
    TimerHandle_t handle = xTimerCreateStatic(timerName, periodMS, pdTRUE, (void *)0, callback, &timerBuffer);

    if (handle != nullptr)
    {
        systemTimers[nextTimerID] = handle;
        return nextTimerID++;
    }
    else
    {
        return -1;
    }
}

void System_Utils::deleteTimer(int timerID)
{
#if DEBUG == 1
    Serial.print("Deleting timer: ");
    Serial.println(timerID);
#endif

    if (systemTimers.find(timerID) != systemTimers.end())
    {
        xTimerDelete(systemTimers[timerID], 0);
        systemTimers.erase(timerID);
    }
}

bool System_Utils::isTimerActive(int timerID)
{
#if DEBUG == 1
    Serial.print("Checking if timer is active: ");
    Serial.println(timerID);
#endif
    if (systemTimers.find(timerID) != systemTimers.end())
    {
        return xTimerIsTimerActive(systemTimers[timerID]);
    }
    return false;
}

void System_Utils::startTimer(int timerID)
{
#if DEBUG == 1
    Serial.print("Starting timer: ");
    Serial.println(timerID);
#endif

    if (systemTimers.find(timerID) != systemTimers.end())
    {
        xTimerStart(systemTimers[timerID], 0);
    }
}

void System_Utils::stopTimer(int timerID)
{
#if DEBUG == 1
    Serial.print("Stopping timer: ");
    Serial.println(timerID);
#endif

    if (systemTimers.find(timerID) != systemTimers.end())
    {
        xTimerStop(systemTimers[timerID], 0);
    }
}

void System_Utils::resetTimer(int timerID)
{
#if DEBUG == 1
    Serial.print("Resetting timer: ");
    Serial.println(timerID);
#endif

    if (systemTimers.find(timerID) != systemTimers.end())
    {
        xTimerReset(systemTimers[timerID], 0);
    }
}

void System_Utils::changeTimerPeriod(int timerID, size_t timerPeriodMS)
{
#if DEBUG == 1
    Serial.print("Changing timer period: ");
    Serial.println(timerID);
#endif

    if (systemTimers.find(timerID) != systemTimers.end())
    {
        xTimerChangePeriod(systemTimers[timerID], pdMS_TO_TICKS(timerPeriodMS), 0);
    }
}

// Queue functionality

int System_Utils::registerQueue(size_t queueLength, size_t itemSize)
{
    QueueHandle_t handle = xQueueCreate(queueLength, itemSize);

    if (handle != nullptr)
    {
        systemQueues[nextQueueID] = handle;
        return nextQueueID++;
    }
    else
    {
        return -1;
    }
}

int System_Utils::registerQueue(size_t queueLength, size_t itemSize, uint8_t *queueBuffer, StaticQueue_t &queueBuffer)
{
    QueueHandle_t handle = xQueueCreateStatic(queueLength, itemSize, queueBuffer, &queueBuffer);

    if (handle != nullptr)
    {
        systemQueues[nextQueueID] = handle;
        return nextQueueID++;
    }
    else
    {
        return -1;
    }
}

QueueHandle_t System_Utils::getQueue(int queueID)
{
    if (systemQueues.find(queueID) != systemQueues.end())
    {
        return systemQueues[queueID];
    }
    else
    {
        return nullptr;
    }
}

void System_Utils::deleteQueue(int queueID)
{
    if (systemQueues.find(queueID) != systemQueues.end())
    {
        vQueueDelete(systemQueues[queueID]);
        systemQueues.erase(queueID);
    }
}

void System_Utils::resetQueue(int queueID)
{
    if (systemQueues.find(queueID) != systemQueues.end())
    {
        xQueueReset(systemQueues[queueID]);
    }
}

bool System_Utils::sendToQueue(int queueID, void *item, size_t timeoutMS)
{
    if (systemQueues.find(queueID) != systemQueues.end())
    {
        return xQueueSend(systemQueues[queueID], item, pdMS_TO_TICKS(timeoutMS)) == pdPASS;
    }
    else
    {
        return false;
    }
}

// Task functionality

int System_Utils::registerTask(
    TaskFunction_t taskFunction, 
    const char *taskName, 
    uint32_t taskStackSize, 
    void *taskParameters, 
    UBaseType_t taskPriority)
{
    TaskHandle_t handle;
    BaseType_t status = xTaskCreate(taskFunction, taskName, taskStackSize, taskParameters, taskPriority, &handle);

    if (status == pdPASS)
    {
        // Add task to systemTasks
        systemTasks[nextTaskID] = handle;
        return nextTaskID++;
    }
    else
    {
        return -1;
    }
}

int System_Utils::registerTask(
    TaskFunction_t taskFunction, 
    const char *taskName, 
    uint32_t taskStackSize, 
    void *taskParameters, 
    UBaseType_t taskPriority, 
    BaseType_t coreID)
{
    TaskHandle_t handle;
    BaseType_t status = xTaskCreatePinnedToCore(taskFunction, taskName, taskStackSize, taskParameters, taskPriority, &handle, coreID);

    if (status == pdPASS)
    {
        // Add task to systemTasks
        systemTasks[nextTaskID] = handle;
        return nextTaskID++;
    }
    else
    {
        return -1;
    }
    
}

int System_Utils::registerTask(
    TaskFunction_t taskFunction, 
    const char *taskName, 
    uint32_t taskStackSize, 
    void *taskParameters, 
    UBaseType_t taskPriority, 
    StackType_t &stackBuffer, 
    StaticTask_t &taskBuffer)
{
    auto handle = xTaskCreateStatic(taskFunction, taskName, taskStackSize, taskParameters, taskPriority, &stackBuffer, &taskBuffer);

    if (handle != nullptr)
    {
        // Add task to systemTasks
        systemTasks[nextTaskID] = handle;
        return nextTaskID++;
    }
    else
    {
        return -1;
    }

}

int System_Utils::registerTask(
    TaskFunction_t taskFunction, 
    const char *taskName, 
    uint32_t taskStackSize, 
    void *taskParameters, 
    UBaseType_t taskPriority, 
    StackType_t &stackBuffer, 
    StaticTask_t &taskBuffer, 
    BaseType_t coreID)
{
    auto handle = xTaskCreateStaticPinnedToCore(taskFunction, taskName, taskStackSize, taskParameters, taskPriority, &stackBuffer, &taskBuffer, coreID);

    if (handle != nullptr)
    {
        // Add task to systemTasks
        systemTasks[nextTaskID] = handle;
        return nextTaskID++;
    }
    else
    {
        return -1;
    }
}

void System_Utils::suspendTask(int taskID)
{
    if (systemTasks.find(taskID) != systemTasks.end())
    {
        vTaskSuspend(systemTasks[taskID]);
    }
}

void System_Utils::resumeTask(int taskID)
{
    if (systemTasks.find(taskID) != systemTasks.end())
    {
        vTaskResume(systemTasks[taskID]);
    }
}

void System_Utils::deleteTask(int taskID)
{
    if (systemTasks.find(taskID) != systemTasks.end())
    {
        vTaskDelete(systemTasks[taskID]);
        systemTasks.erase(taskID);
    }
}

bool System_Utils::enableWiFi()
{
    enableRadio(ADC_WIFI);
    WiFi.disconnect(false);  // Reconnect the network
    WiFi.mode(WIFI_STA);    // Switch WiFi off
 
    WiFi.begin("ESP32-OTA", "e65v41ev");
 
    size_t timeoutCounter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500));
        if (timeoutCounter++ > 20)
        {
            return false;
        }
    }

    return true;
}

#ifdef USE_BLE

void System_Utils::initBluetooth()
{
    #if DEBUG == 1
    Serial.println("Initializing Bluetooth");
    #endif
    esp_err_t ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);


    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    ret = esp_bluedroid_init();
    ret = esp_bluedroid_enable();


    #if DEBUG == 1
    Serial.println("Registering event handlers");
    #endif

    ret = esp_ble_gatts_register_callback(gattsEventHandler);
    ret = esp_ble_gap_register_callback(gapEventHandler);

    

    #if DEBUG == 1
    Serial.println("Bluetooth initialized");
    #endif
}

void System_Utils::gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    #if DEBUG == 1
    Serial.print("GAP Event: ");
    Serial.println(event);
    #endif
}

void System_Utils::gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    #if DEBUG == 1
    Serial.print("GATTS Event: ");
    Serial.println(event);
    #endif
}

#endif

// void System_Utils::enableBluetooth()
// {
//     enableRadio(ADC_BT);
//     esp_bt_controller_enable(ESP_BT_MODE_BTDM);
// }

// void System_Utils::disableBluetooth()
// {
//     disableRadio(ADC_BT);
//     esp_bt_controller_disable();
// }

// void System_Utils::addCharacteristic(BLECharacteristic &characteristic)
// {
//     pService->addCharacteristic(&characteristic);
//     bleCharacteristics.push_back(&characteristic);
// }

void System_Utils::disableWiFi()
{
    disableRadio(ADC_WIFI);
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
}

IPAddress System_Utils::getLocalIP()
{
    return WiFi.localIP();
}

void System_Utils::enableRadio(uint8_t adcUser)
{
    if (adcUsers.find(adcUser) != adcUsers.end() && !adcUsers[adcUser])
    {
        adcUsers[adcUser] = true;
        adc_power_acquire();
    }
}

void System_Utils::disableRadio(uint8_t adcUser)
{
    if (adcUsers.find(adcUser) != adcUsers.end() && adcUsers[adcUser])
    {
        adcUsers[adcUser] = false;
        adc_power_release();
    }
}

bool System_Utils::initializeOTA()
{
    if (otaInitialized)
    {
        return false;
    }

    ArduinoOTA.setHostname("ESP32-OTA");
    ArduinoOTA.setPort(8266);

    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

    otaTaskID = registerTask([](void *pvParameters) {
        for (;;)
        {
            ArduinoOTA.handle();
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }, "OTA Handler", 8192, nullptr, 1);

    otaInitialized = true;

    return true;
}

void System_Utils::startOTA()
{
    if (!otaInitialized)
    {
        initializeOTA();
    }

    ArduinoOTA.begin();
    resumeTask(otaTaskID);
}

void System_Utils::stopOTA()
{
    suspendTask(otaTaskID);
    ArduinoOTA.end();
}

void System_Utils::sendDisplayContents(Adafruit_SSD1306 *display)
{
    DynamicJsonDocument doc(10000);
    auto buffer = display->getBuffer();

    size_t bufferLength = (display->width() * display->height()) / 8;

    doc[COMMAND_FIELD] = (int)DISPLAY_CONTENTS;
    auto displayBuffer = doc.createNestedArray(DISPLAY_BUFFER_FIELD);

    doc[DISPLAY_WIDTH] = display->width();
    doc[DISPLAY_HEIGHT] = display->height();

    for (size_t i = 0; i < display->height() / 8; i++)
    {
        doc[DISPLAY_BUFFER_FIELD].createNestedArray();
    }

    for (size_t i = 0; i < display->width(); i++)
    {
        for (size_t j = 0; j < display->height() / 8; j++)
        {
            displayBuffer[j].add(buffer[i * (display->height() / 8) + j]);
        }
    }

    Serial.printf("Row size: %d\n", displayBuffer[0].size());

    ArduinoJson::serializeJson(doc, Serial);
}