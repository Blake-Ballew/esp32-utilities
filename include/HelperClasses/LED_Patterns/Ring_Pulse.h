#pragma once

#include "LED_Pattern.h"

// Fades in entire LED ring then fades out
class Ring_Pulse : public LED_Pattern
{
public:
    Ring_Pulse() 
    {

    }

    void configurePattern(JsonObject &config)
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
    }

    bool iterateFrame()
    {
        // Determine brightness this frame
        // First half of animation fades in, second half fades out
        float brightness = 1.0;
        if (currTick < animationTicks / 2)
        {
            brightness = (float)currTick / (float)(animationTicks / 2);
        }
        else
        {
            brightness = 1.0 - (float)(currTick - (animationTicks / 2)) / (float)(animationTicks / 2);
        }

        // use override colors if set
        CRGB color;
        if (rOverride != 0 || gOverride != 0 || bOverride != 0)
        {
            color = CRGB(rOverride * brightness, gOverride * brightness, bOverride * brightness);
        }
        else
        {
            color = CRGB(r * brightness, g * brightness, b * brightness);
        }

        // Set the color for the entire ring
        for (uint32_t i = beginIdx; i < endIdx; i++)
        {
            leds[i] = color;
        }

        currTick++;

        if (currTick == animationTicks)
        {
            // Reset the pattern
            resetPattern();
            return true;
        }
        else
        {
            return false;
        }
    }

    void setRegisteredPatternID(int patternID) { registeredPatternID = patternID; }
    int registeredPatternID() { return registeredPatternID; }

protected:
    static int registeredPatternID;

    // Override colors
    uint8_t rOverride, gOverride, bOverride;

    // Beginning/end of the ring
    uint32_t beginIdx, endIdx;
};