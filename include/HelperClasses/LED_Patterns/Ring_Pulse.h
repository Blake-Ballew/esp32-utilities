#pragma once

#include "LED_Pattern_Interface.h"

// Fades in entire LED ring then fades out
class Ring_Pulse : public LED_Pattern_Interface
{
public:
    Ring_Pulse() 
    {
        rOverride = 0;
        gOverride = 0;
        bOverride = 0;

        beginIdx = -1;
        endIdx = -1;
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
    }

    bool iterateFrame()
    {
        if (beginIdx == -1 || endIdx == -1)
        {
            return true;
        }

        if (startTime == 0) 
        {
            startTime = xTaskGetTickCount();
        }

        size_t currMS = xTaskGetTickCount() - startTime;

        // Determine brightness this frame
        // First half of animation fades in, second half fades out
        float brightness = 1.0;
        if (currMS < animationMS / 2)
        {
            brightness = (float)currMS / (float)(animationMS / 2);
        }
        else
        {
            brightness = 1.0 - (float)(currMS - (animationMS / 2)) / (float)(animationMS / 2);
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

        if (currMS >= animationMS)
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

    void clearPattern()
    {
        if (beginIdx == -1 || endIdx == -1)
        {
            return;
        }

        for (uint32_t i = beginIdx; i < endIdx; i++)
        {
            leds[i] = CRGB(0, 0, 0);
        }

        resetPattern();
    }

    void SetRegisteredPatternID(int patternID) { registeredPatternID = patternID; }
    static int RegisteredPatternID() { return registeredPatternID; }

protected:
    static int registeredPatternID;

    // Override colors
    uint8_t rOverride, gOverride, bOverride;

    // Beginning/end of the ring
    int beginIdx, endIdx;
};