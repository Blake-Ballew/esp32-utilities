#include "GPS_Content.h"

GPS_Content::GPS_Content()
{
}

GPS_Content::~GPS_Content()
{
}

void GPS_Content::encUp()
{
}

void GPS_Content::encDown()
{
}

void GPS_Content::printContent()
{
    display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

    NavigationUtils::UpdateGPS();
    TinyGPSLocation loc = NavigationUtils::GetLocation();

    display->setCursor(0, 8);
    display->print(" Lat:");
    ESP_LOGV(TAG, "Lat: %f", loc.lat());
    display->print(loc.lat(), 10);
    display->setCursor(0, 16);
    ESP_LOGV(TAG, "Long: %f", loc.lng());
    display->print("Long:");
    display->print(loc.lng(), 10);

    display->display();
}