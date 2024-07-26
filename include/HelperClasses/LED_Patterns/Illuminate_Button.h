#pragma once

#include "LED_Pattern.h"

class Illuminate_Button : LED_Pattern_Interface
{
public:
    Illuminate_Button(std::vector<std::pair<uint8_t, uint8_t>> inputIDLedIdx)
    {
        for (auto kvp : inputIDLedIdx)
        {
            inputIdLedPins[kvp.first] = kvp.second;
            inputIdState[kvp.first] = false;
        }

        animationTicks = 15;
    }

    void configurePattern(JsonObject &config)
    {
        if (config.containsKey("inputStates"))
        {
            for (auto kvp : config["inputStates"].as<JsonArray>())
            {
                if (kvp.containsKey("input") && kvp.containsKey("state"))
                {
                    inputIdState[kvp["input"]] = kvp["state"];
                }
            }
        }
    }

    bool iterateFrame()
    {
        for (auto kvp : inputIdLedPins)
        {
            if (inputIdState[kvp.first])
            {
                leds[kvp.second] = CRGB(r, g, b);
            }
            else
            {
                leds[kvp.second] = CRGB(0, 0, 0);
            }
        }

        return true;
    }

    void clearPattern()
    {
        for (auto kvp : inputIdLedPins)
        {
            leds[kvp.second] = CRGB(0, 0, 0);
        }

        resetPattern();
    }

    void setRegisteredPatternID(int patternID) { registeredPatternID = patternID; }
    int registeredPatternID() { return registeredPatternID; }

protected:
    std::unordered_map<uint8_t, uint8_t> inputIdLedPins;
    std::unordered_map<uint8_t, bool> inputIdState;
    uint8_t inputID;

    static int registeredPatternID;
};