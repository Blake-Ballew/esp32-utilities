#pragma once

#include "Window_State.h"
#include <vector>
#include "LoraUtils.h"
#include "LED_Utils.h"
#include "RingPoint.h"
#include "NavigationUtils.h"

namespace
{
    const size_t LedRingIdxStart = 0;
    const size_t LedRingIdxEnd = 15;
}

class Tracking_State : public Window_State
{
public:
    Tracking_State()
    {
        allowInterrupts = false;
    }

    ~Tracking_State()
    {
        
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        Display_Utils::enableRefreshTimer(100);

        _RingPointID = RingPoint::RegisteredPatternID();
        LED_Utils::enablePattern(_RingPointID);

        if (transferData.serializedData != nullptr)
        {
            auto doc = transferData.serializedData;

            StaticJsonDocument<256> cfg;

            r = (*doc)["color_R"];
            g = (*doc)["color_G"];
            b = (*doc)["color_B"];

            lat = (*doc)["lat"];
            lng = (*doc)["lon"];

            for (auto it : (*doc)["displayTxt"].as<JsonArray>())
            {
                displayText.push_back(it.as<std::string>());
            }

            cfg["rOverride"] = r;
            cfg["gOverride"] = g;
            cfg["bOverride"] = b;

            LED_Utils::configurePattern(_RingPointID, cfg);
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        Display_Utils::disableRefreshTimer();
        vTaskDelay(pdMS_TO_TICKS(100));
        LED_Utils::disablePattern(_RingPointID);

        displayText.clear();
    }

    void displayState()
    {
        // #if DEBUG != 1
        // if (!NavigationUtils::IsGPSConnected())
        // {
        //     Display_Utils::printCenteredText("No GPS Signal");
        //     display->display();
        //     Display_Utils::sendCallbackCommand(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE);
        //     vTaskDelay(pdMS_TO_TICKS(1000));
        //     return;
        // }
        // #endif

        Window_State::displayState();

        double distance = NavigationUtils::GetDistanceTo(lat, lng);

        char distanceStr[10];
        if (distance > 2000)
        {
            sprintf(distanceStr, "%.1f km", distance / 1000);
        }
        else
        {
            sprintf(distanceStr, "%d m", (uint32_t)distance);
        }

        TextFormat distanceFormat;
        distanceFormat.horizontalAlignment = ALIGN_RIGHT;
        distanceFormat.verticalAlignment = TEXT_LINE;
        distanceFormat.line = 1;
        Display_Utils::printFormattedText(distanceStr, distanceFormat);

        TextFormat format;
        format.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
        format.verticalAlignment = TEXT_LINE;
        uint8_t textLine = 2;

        for (auto str : displayText)
        {
            format.line = textLine;
            Display_Utils::printFormattedText(str.c_str(), format);
            textLine++;
        }

        size_t ledFxMin = 20;
        size_t ledFxMax = 500;

        if (distance > ledFxMax)
        {
            distance = ledFxMax;
        } 
        else if (distance < ledFxMin)
        {
            distance = ledFxMin;
        }

        double heading = NavigationUtils::GetHeadingTo(lat, lng);
        int azimuth = NavigationUtils::GetAzimuth();

        float fadeDegrees = -0.075f * distance + 61.5;
        float directionDegrees = heading - azimuth;

        if (directionDegrees < 0)
        {
            directionDegrees += 360;
        }

        StaticJsonDocument<64> cfg;
        cfg["fadeDegrees"] = fadeDegrees;
        cfg["directionDegrees"] = directionDegrees;

        #if DEBUG == 1
            // Serial.print("heading: ");
            // Serial.println(heading);
            // Serial.print("azimuth: ");
            // Serial.println(azimuth);
        #endif

        LED_Utils::configurePattern(_RingPointID, cfg);
        LED_Utils::iteratePattern(_RingPointID);

        display->display();
    }

private:
    // Text to display in window
    std::vector<std::string> displayText;

    // Coordinates to track
    double lat = 0;
    double lng = 0;

    // Color of ring point
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    int _RingPointID = -1;
};