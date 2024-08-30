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

        #if DEBUG == 1
            // Serial.println("ScrollWheel::iterateFrame");
            // Serial.print("beginIdx: ");
            // Serial.print(beginIdx);
            // Serial.print(" endIdx: ");
            // Serial.print(endIdx);
            // Serial.print(" numItems: ");
            // Serial.print(numItems);
            // Serial.print(" currItem: ");
            // Serial.println(currItem);
        #endif

        for (int i = beginIdx; i <= endIdx; i++)
        {
            float ledBrightness = _CalculateLEDPointBrightness(i);
            leds[i] = CRGB(LED_Pattern_Interface::r * ledBrightness, LED_Pattern_Interface::g * ledBrightness, LED_Pattern_Interface::b * ledBrightness);
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

        #if DEBUG == 1
            // Serial.print("ledIdx: ");
            // Serial.print(ledIdx);
        #endif

        #if DEBUG == 1
            // Serial.print(" currItem: ");
            // Serial.print(currItem);
            // Serial.print(" numItems: ");
            // Serial.print(numItems);
        #endif

        // Target angle of the LED in degrees
        float targetAngle = ((float)currItem / numItems) * 360.0;
        #if DEBUG == 1
            // Serial.print(" targetAngle: ");
            // Serial.print(targetAngle);
        #endif

        float fadeDegrees = (360.0 / (float)numItems) / 2.0;

        // Calculate the angle of the LED from the beginning index
        float angle = (float)(ledIdx - beginIdx) * 360.0 / (float)(endIdx - beginIdx);

        #if DEBUG == 1
            // Serial.print(" LEDangle: ");
            // Serial.print(angle);
        #endif

        // Calculate the angle difference between the LED and the target angle
        float angleDiff = abs(angle - targetAngle);

        // If the angle difference is greater than 180, use the smaller angle
        if (angleDiff > 180)
        {
            angleDiff = 360 - angleDiff;
        }

        #if DEBUG == 1
            // Serial.print(" angleDiff: ");
            // Serial.print(angleDiff);
        #endif

        // If the angle difference is greater than the fade degrees, return 0
        if (angleDiff > fadeDegrees)
        {
            return 0;
        }

        // Calculate the brightness based on the angle difference with exponential decay
        float returnVal = (-1.0 * pow(angleDiff / fadeDegrees, 2.0)) + 1.0;
        #if DEBUG == 1
            // Serial.print(" returnVal: ");
            // Serial.print(returnVal);
            // Serial.print(" fadeDegrees: ");
            // Serial.println(fadeDegrees);
        #endif

        return returnVal;
    }
};