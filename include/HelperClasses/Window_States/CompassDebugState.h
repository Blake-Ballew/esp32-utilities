#pragma once

#include "Window_State.h"

#include "NavigationUtils.h"
#include "LED_Utils.h"

#include "RingPoint.h"

namespace
{
    const size_t COMPASS_WINDOW_REFRESH_RATE_MS = 100;
}

class CompassDebugState : public Window_State
{
public:
    CompassDebugState() {}
    ~CompassDebugState() {}

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);
        
        _RingPointID = RingPoint::RegisteredPatternID();

        StaticJsonDocument<128> cfg;

        cfg["rOverride"] = LED_Utils::ThemeColor().r;
        cfg["gOverride"] = LED_Utils::ThemeColor().g;
        cfg["bOverride"] = LED_Utils::ThemeColor().b;

        LED_Utils::configurePattern(_RingPointID, cfg);

        LED_Utils::enablePattern(_RingPointID);

        Display_Utils::enableRefreshTimer(COMPASS_WINDOW_REFRESH_RATE_MS);
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);
        LED_Utils::disablePattern(_RingPointID);
        Display_Utils::disableRefreshTimer();
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    void displayState()
    {
        float azimuth = NavigationUtils::GetAzimuth();

        float northDirection = 360.0f - azimuth;
        float pointerWidthDeg = 25.0f;

        StaticJsonDocument<64> cfg;

        cfg["directionDegrees"] = northDirection;
        cfg["fadeDegrees"] = pointerWidthDeg;

        LED_Utils::configurePattern(_RingPointID, cfg);
        LED_Utils::iteratePattern(_RingPointID);

        char displaybuffer[23];

        NavigationUtils::PrintRawValues();

        sprintf(displaybuffer, "Azimuth: %.1f", azimuth);
        Display_Utils::printCenteredText(displaybuffer);

        Display_Utils::UpdateDisplay().Invoke();
    }


protected:
    int _RingPointID;
};