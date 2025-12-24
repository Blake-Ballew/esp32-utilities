#pragma once

#include "LED_Pattern_Interface.h"


// A pattern used to point a direction with the LED ring.
// The angle is degrees from the beginning index of the ring is supplied for the direction.
// The sharpness of the point is controlled by the fadeDegrees parameter.
// This can be used as a compass or a scrolling indicator in menus.
// This pattern is not animated and will be called when updates are needed.
class RingPoint : public LED_Pattern_Interface
{
public:

    RingPoint() 
    {
        rOverride = 0;
        gOverride = 0;
        bOverride = 0;

        beginIdx = -1;
        endIdx = -1;

        fadeDegrees = 0;
        directionDegrees = 0;

        setAnimationLengthTicks(1);
    }

    void configurePattern(JsonDocument &config)
    {
        if (config.containsKey("beginIdx"))
        {
            beginIdx = config["beginIdx"];
        }

        if (config.containsKey("endIdx"))
        {
            endIdx = config["endIdx"];
        }

        if (config.containsKey("rOverride"))
        {
            rOverride = config["rOverride"];
        }

        if (config.containsKey("gOverride"))
        {
            gOverride = config["gOverride"];
        }

        if (config.containsKey("bOverride"))
        {
            bOverride = config["bOverride"];
        }

        if (config.containsKey("fadeDegrees"))
        {
            fadeDegrees = config["fadeDegrees"];
        }

        if (config.containsKey("directionDegrees"))
        {
            directionDegrees = config["directionDegrees"];
        }
    }

    bool iterateFrame()
    {
        ESP_LOGV(TAG, "RingPoint::iterateFrame");
        if (beginIdx == -1 || endIdx == -1)
        {
            ESP_LOGW(TAG, "RingPoint::iterateFrame: Invalid begin or end index");
            return true;
        }

        // Clear ring
        for (int i = beginIdx; i <= endIdx; i++)
        {
            leds[i] = CRGB(0, 0, 0);
        }

        CRGB outColor;

        if (rOverride == 0 && gOverride == 0 && bOverride == 0)
        {
            outColor = themeColor;
        }
        else
        {
            outColor = CRGB(rOverride, gOverride, bOverride);
        }

        for (int i = beginIdx; i <= endIdx; i++)
        {
            float brightness = GetLEDPointBrightness(i);

            leds[i] = CRGB(outColor.r * brightness, outColor.g * brightness, outColor.b * brightness);
        }

        return true;
    }

    void clearPattern()
    {
        if (beginIdx == -1 || endIdx == -1)
        {
            return;
        }

        for (int i = beginIdx; i <= endIdx; i++)
        {
            leds[i] = CRGB(0, 0, 0);
        }

        FastLED.show();
    }

    void SetRegisteredPatternID(int patternID) { _RegisteredPatternID = patternID; }
    static int RegisteredPatternID() { return _RegisteredPatternID; }

protected:
    static int _RegisteredPatternID;

    uint8_t rOverride;
    uint8_t gOverride;
    uint8_t bOverride;

    int beginIdx;
    int endIdx;

    float fadeDegrees;
    float directionDegrees;

    const char *TAG = "RingPoint";
    
    float GetLEDPointBrightness(int ledIdx)
    {
        if (beginIdx == -1 || endIdx == -1)
        {
            return 0;
        }

        // Calculate the angle of the LED from the beginning index
        float angle = (float)(ledIdx - beginIdx) * 360.0 / (float)(endIdx - beginIdx);

        // Calculate the angle difference between the LED and the direction
        float angleDiff = abs(angle - directionDegrees);

        // If the angle difference is greater than 180, use the smaller angle
        if (angleDiff > 180)
        {
            angleDiff = 360 - angleDiff;
        }

        // If the angle difference is greater than the fade degrees, return 0
        if (angleDiff > fadeDegrees)
        {
            return 0;
        }

        // Calculate the brightness based on the angle difference
        return 1.0 - (angleDiff / fadeDegrees);
    }
};