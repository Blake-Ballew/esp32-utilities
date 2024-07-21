#pragma once

#include <FastLED.h>
#include "globalDefines.h"
#include "LED_Pattern_Interface.h"
#include <unordered_map>

// Called when an input with an LED is pressed. LED turns on and fades away over the course of the animation length
class Button_Flash : LED_Pattern_Interface
{
public:
    Button_Flash()
    {
        r = 0;
        g = 0;
        b = 0;

        animationTicks = 15;
    }

    void configurePattern(JsonObject &config)
    {
        if (config.containsKey("inputID"))
        {
            inputID = config["inputID"];
        }
        if (config.containsKey("inputIdLedPins"))
        {
            inputIdLedPins.clear();

            auto inputIdLedPinsConfig = config["inputIdLedPins"].as<JsonArray>();
            for (auto kvp : inputIdLedPinsConfig)
            {
                auto kvpObj = kvp.as<JsonObject>();
                inputIdLedPins[kvpObj["inputId"]] = kvpObj["ledPin"];
            }
        }
    }

    bool iterateFrame()
    {
        if (inputIdLedPins.find(inputID) != inputIdLedPins.end())
        {
            // Determine brightness this frame
            float brightness = 1.0 - (float)currTick / (float)animationTicks;
            if (inputIdLedPins[inputID] < numLeds) 
            {
                leds[inputIdLedPins[inputID]] = CRGB(r * brightness, g * brightness, b * brightness);
            }
        }
        else 
        {
            resetPattern();
            return true;
        }

        currTick++;

        if (currTick == animationTicks)
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

protected:
    std::unordered_map<uint8_t, uint8_t> inputIdLedPins;
    uint8_t inputID;
};