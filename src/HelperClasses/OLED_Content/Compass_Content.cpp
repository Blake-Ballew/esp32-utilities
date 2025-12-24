#include "Compass_Content.h"

Compass_Content *Compass_Content::thisInstance = nullptr;
StaticTimer_t Compass_Content::updateTimerBuffer;
TimerHandle_t Compass_Content::updateTimer = xTimerCreateStatic("CompassUpdate", pdMS_TO_TICKS(25), pdTRUE, (void *)0, updateCompass, &updateTimerBuffer);

Compass_Content::Compass_Content(Adafruit_SSD1306 *disp)
{
    display = disp;
    thisInstance = this;
    xTimerStart(this->updateTimer, 0);
}

Compass_Content::~Compass_Content()
{
    ESP_LOGV(TAG, "Compass_Content destructor");
    if (xTimerIsTimerActive(updateTimer) == pdTRUE)
    {
        ESP_LOGV(TAG, "Compass_Content destructor: Timer is active");
        if (xTimerStop(updateTimer, 0) == pdFALSE)
        {
            ESP_LOGV(TAG, "Compass_Content destructor: Could not stop timer");
        }
        else
        {
            ESP_LOGV(TAG, "Compass_Content destructor: Timer stopped");
        }
    }
    /*         if (xTimerDelete(updateTimer, 500) == pdFALSE)
            {
                ESP_LOGV(TAG, "Compass_Content destructor: Could not delete timer");
            } */
    ESP_LOGV(TAG, "Compass_Content destructor: Timer deleted");
    thisInstance = nullptr;
}

void Compass_Content::printContent()
{

    display->fillRect(0, 0, OLED_WIDTH, OLED_HEIGHT - 8, BLACK);

    display->setCursor(0, 0);
    display->print("X:");
    display->print(NavigationUtils::GetX());

    display->setCursor(0, 8);
    display->print("Y:");
    display->print(NavigationUtils::GetY());

    display->setCursor(0, 16);
    display->print("Z:");
    display->print(NavigationUtils::GetZ());

    display->setCursor(40, 0);
    display->print("  Azimuth:");
    display->print(NavigationUtils::GetAzimuth());

    display->display();

    LED_Manager::pointNorth(NavigationUtils::GetAzimuth());
}

void Compass_Content::encUp()
{
}

void Compass_Content::encDown()
{
}

void Compass_Content::stop()
{
    xTimerStop(this->updateTimer, 0);
}

void Compass_Content::start()
{
    xTimerStart(this->updateTimer, 0);
}

void Compass_Content::updateCompass(TimerHandle_t xTimer)
{
    if (thisInstance == nullptr)
    {
        xTimerStop(updateTimer, 0);
        return;
    }

    thisInstance->printContent();
}
