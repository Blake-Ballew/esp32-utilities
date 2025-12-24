#pragma once

#include <FastLED.h>
#include "globalDefines.h"
#include "LED_Pattern_Interface.h"
#include <unordered_map>
#include <vector>
// #include <utilities>

// Called when an input with an LED is pressed. LED turns on and fades away over the course of the animation length
class Button_Flash : public LED_Pattern_Interface
{
public:
    Button_Flash(std::unordered_map<uint8_t, uint8_t> inputIDLedIdx)
    {
        for (auto kvp : inputIDLedIdx)
        {
            inputIdLedPins[kvp.first] = kvp.second;
        }

        animationTicks = 15;
    }

    void configurePattern(JsonDocument &config)
    {
        if (config.containsKey("inputID"))
        {
            inputID = config["inputID"];
        }

        clearPattern();
    }

    bool iterateFrame()
    {
        if (startTime == 0) 
        {
            startTime = xTaskGetTickCount();
        }

        if (inputIdLedPins.find(inputID) != inputIdLedPins.end())
        {
            // Determine brightness this frame
            float brightness = 1.0 - (float)(xTaskGetTickCount() - startTime) / (float)animationMS;
            if (inputIdLedPins[inputID] < numLeds) 
            {
                leds[inputIdLedPins[inputID]] = CRGB(themeColor.r * brightness, themeColor.g * brightness, themeColor.b * brightness);
            }
        }
        else 
        {
            resetPattern();
            return true;
        }

        currTick++;

        if ((xTaskGetTickCount() - startTime) >= animationMS)
        {
            if (inputIdLedPins[inputID] < numLeds) 
            {
                leds[inputIdLedPins[inputID]] = CRGB(0, 0, 0);
            }

            resetPattern();
            return true;
        } 
        else
        {
            return false;
        }
    }

    void clearPattern()
    {
        for (auto kvp : inputIdLedPins)
        {
            if (kvp.second < numLeds)
            {
                leds[kvp.second] = CRGB(0, 0, 0);
            }
        }

        resetPattern();
    }

    void SetRegisteredPatternID(int patternID) { registeredPatternID = patternID; }
    static int RegisteredPatternID() { return registeredPatternID; }

protected:
    std::unordered_map<uint8_t, uint8_t> inputIdLedPins;
    uint8_t inputID;

    static int registeredPatternID;
    const char *TAG = "Button_Flash";
};