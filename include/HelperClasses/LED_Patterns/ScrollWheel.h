#pragma once

#include "LED_Pattern_Interface.h"


// Uses the LED ring to display scrolling progress
class ScrollWheel : public LED_Pattern_Interface
{
public:
    ScrollWheel()
    {
        beginIdx = -1;
        endIdx = -1;

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

        if (config.containsKey("numItems"))
        {
            numItems = config["numItems"];
        }

        if (config.containsKey("currItem"))
        {
            currItem = config["currItem"];
        }
    }

    bool iterateFrame()
    {
        if (beginIdx == -1 || endIdx == -1 || numItems == -1 || currItem == -1)
        {
            return true;
        }

        ESP_LOGV(TAG, "ScrollWheel::iterateFrame beginIdx: %d endIdx: %d numItems: %d currItem: %d", beginIdx, endIdx, numItems, currItem);

        for (int i = beginIdx; i <= endIdx; i++)
        {
            float ledBrightness = _CalculateLEDPointBrightness(i);
            leds[i] = CRGB(themeColor.r * ledBrightness, themeColor.g * ledBrightness, themeColor.b * ledBrightness);
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

    // LED ring beginning and ending indices
    int beginIdx;
    int endIdx;

    // Number of scrolling items to represent
    int numItems;

    // Current scrolling item
    int currItem;

    // Calculate the brightness of the LED at the given index
    float _CalculateLEDPointBrightness(int ledIdx)
    {
        if (beginIdx == -1 || endIdx == -1 || numItems == -1 || currItem == -1)
        {
            return 0;
        }

        // Target angle of the LED in degrees
        float targetAngle = ((float)currItem / numItems) * 360.0;

        float fadeDegrees = max((360.0f / (float)numItems) / 2.0f, (360.0f / (endIdx - beginIdx)));

        // Calculate the angle of the LED from the beginning index
        float angle = (float)(ledIdx - beginIdx) * 360.0 / (float)(endIdx - beginIdx);

        // Calculate the angle difference between the LED and the target angle
        float angleDiff = abs(angle - targetAngle);

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

        // Calculate the brightness based on the angle difference with exponential decay
        float returnVal = (-1.0 * pow(angleDiff / fadeDegrees, 2.0)) + 1.0;

        ESP_LOGV(TAG, "ledIdx: %d currItem: %d numItems: %d targetAngle: %f LEDangle: %f angleDiff: %f returnVal: %f fadeDegrees: %f",
                 ledIdx, currItem, numItems, targetAngle, angle, angleDiff, returnVal, fadeDegrees);

        return returnVal;
    }
};