#pragma once

#include "Window_State.h"
#include "LoraMessageDisplay.h"
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
    Tracking_State(LoraMessageDisplay *content)
    {
        messageDisplay = content;
        renderContent = content;
    }

    ~Tracking_State()
    {
        
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        Display_Utils::enableRefreshTimer(50);

        if (RingPoint::RegisteredPatternID() == -1)
        {
            RingPoint *ringPoint = new RingPoint();
            LED_Utils::registerPattern(ringPoint);
        }

        _RingPointID = RingPoint::RegisteredPatternID();
        LED_Utils::enablePattern(_RingPointID);

        StaticJsonDocument<256> cfg;
        cfg["beginIdx"] = LedRingIdxStart;
        cfg["endIdx"] = LedRingIdxEnd;

        if (transferData.serializedData != nullptr)
        {
            auto doc = transferData.serializedData;

            if (MessageBase::GetMessageTypeFromJson(*doc) == MessagePing::MessageType())
            {
                pingMsg = new MessagePing();
                pingMsg->deserialize(*doc);
                messageDisplay->SetDisplayMessage(pingMsg);

                cfg["rOverride"] = pingMsg->color_R;
                cfg["gOverride"] = pingMsg->color_G;
                cfg["bOverride"] = pingMsg->color_B;

                LED_Utils::configurePattern(_RingPointID, cfg);
            }
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        Display_Utils::disableRefreshTimer();
        LED_Utils::disablePattern(_RingPointID);
        LED_Utils::clearPattern(_RingPointID);

        messageDisplay->ClearDisplayMessage();
        pingMsg = nullptr;
    }

    void displayState()
    {
        #if DEBUG != 1
        if (!Navigation_Manager::IsGPSConnected())
        {
            Display_Utils::printCenteredText("No GPS Signal");
            display->display();
            Display_Utils::sendCallbackCommand(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE);
            vTaskDelay(pdMS_TO_TICKS(1000));
            return;
        }
        #endif

        Window_State::displayState();

        if (pingMsg != nullptr)
        {
            double distance = NavigationUtils::GetDistanceTo(pingMsg->lat, pingMsg->lng);

            char distanceStr[10];
            if (distance > 2000)
            {
                sprintf(distanceStr, "%.1f km", distance / 1000);
            }
            else
            {
                sprintf(distanceStr, "%d m", (uint32_t)distance);
            }

            TextFormat format;
            format.horizontalAlignment = ALIGN_RIGHT;
            format.verticalAlignment = TEXT_LINE;
            format.line = 2;

            Display_Utils::printFormattedText(distanceStr, format);

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

            double heading = NavigationUtils::GetHeadingTo(pingMsg->lat, pingMsg->lng);
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
                Serial.print("heading: ");
                Serial.println(heading);
                Serial.print("azimuth: ");
                Serial.println(azimuth);
            #endif

            LED_Utils::configurePattern(_RingPointID, cfg);
            LED_Utils::iteratePattern(_RingPointID);
        }

        display->display();
    }

private:
    LoraMessageDisplay *messageDisplay;
    MessagePing *pingMsg = nullptr;

    int _RingPointID = -1;
};