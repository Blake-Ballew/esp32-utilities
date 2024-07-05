#include "System_Utils.h"

bool System_Utils::silentMode = true;
std::unordered_map<int, TimerHandle_t> System_Utils::systemTimers;
int System_Utils::nextTimerID = 0;
StaticTimer_t System_Utils::healthTimerBuffer;
int System_Utils::healthTimerID;
Adafruit_SSD1306 *System_Utils::OLEDdisplay = nullptr;
System_Utils::otaInitialized = false;

void System_Utils::init(Adafruit_SSD1306 *display)
{
    System_Utils::OLEDdisplay = display;
    healthTimerID = registerTimer("System Health Monitor", 60000, monitorSystemHealth, healthTimerBuffer);
    startTimer(healthTimerID);
    monitorSystemHealth(nullptr);
}
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
    OLEDdisplay->clearDisplay();
    OLEDdisplay->setCursor((OLED_WIDTH / 2) - (11 * 3), 8);
    OLEDdisplay->println("Low Battery");
    OLEDdisplay->setCursor((OLED_WIDTH / 2) - (13 * 3), 16);
    OLEDdisplay->println("Shutting Down");
    OLEDdisplay->display();
    // LED_Manager::ledShutdownAnimation();
    digitalWrite(KEEP_ALIVE_PIN, LOW);
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

void System_Utils::initializeOTA()
{
    // TODO: Include different connection options

    ArduinoOTA.setHostname("ESP32-OTA");

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

    ArduinoOTA.begin();

    auto timerCode = registerTimer("OTA Timer", 1000, [](TimerHandle_t xTimer) {
        ArduinoOTA.handle();
    });

    startTimer(timerCode);

    otaInitialized = true;
}

void System_Utils::stopOTA()
{
    ArduinoOTA.end();
    otaInitialized = false;
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