#include "System_Utils.h"

bool System_Utils::silentMode = true;
std::unordered_map<int, TimerHandle_t> System_Utils::systemTimers;
int System_Utils::nextTimerID = 0;
StaticTimer_t System_Utils::healthTimerBuffer;
TimerHandle_t System_Utils::healthTimer = xTimerCreateStatic("System_Health_Timer", pdMS_TO_TICKS(60000 * 5), pdTRUE, (void *)0, monitorSystemHealth, &healthTimerBuffer);
Adafruit_SSD1306 *System_Utils::display = nullptr;

void System_Utils::init(Adafruit_SSD1306 *display)
{
    System_Utils::display = display;
    xTimerStart(healthTimer, 0);
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

void System_Utils::shutdownBatteryWarning() {
    display->clearDisplay();
    display->setCursor((OLED_WIDTH / 2) - (11 * 3), 8);
    display->println("Low Battery");
    display->setCursor((OLED_WIDTH / 2) - (13 * 3), 16);
    display->println("Shutting Down");
    display->display();
    LED_Manager::ledShutdownAnimation();
    digitalWrite(KEEP_ALIVE_PIN, LOW);
}

int System_Utils::registerTimer(const char * timerName, size_t periodMS, TimerCallbackFunction_t callback) {
    TimerHandle_t handle = xTimerCreate(timerName, periodMS, pdTRUE, (void *)0, callback);

    if (handle != nullptr) {
        systemTimers[nextTimerID] = handle;
        return nextTimerID++;
    } else {
        return -1;
    }
}

int System_Utils::registerTimer(const char * timerName, size_t periodMS, TimerCallbackFunction_t callback, StaticTimer_T &timerBuffer) {
    TimerHandle_t handle = xTimerCreateStatic(timerName, periodMS, pdTRUE, (void *)0, callback, &timerBuffer);

    if (handle != nullptr) {
        systemTimers[nextTimerID] = handle;
        return nextTimerID++;
    } else {
        return -1;
    }
}

void System_Utils::startTimer(int timerID) {
    
}

void System_Utils::stopTimer(int timerID) {

}

void System_Utils::changeTimerPeriod(int timerID, size_t timerPeriodMS) {

}