#pragma once

#include "LED_Pattern_Interface.h"

// Uses the LED ring to display a solid color
class SolidRing : public LED_Pattern_Interface
{
public:
    SolidRing()
    {
        rOverride = 0;
        gOverride = 0;
        bOverride = 0; 

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
        #if DEBUG == 1
            Serial.println("SolidRing::iterateFrame");
        #endif

        if (beginIdx == -1 || endIdx == -1)
        {
            return true;
        }

        #if DEBUG == 1
            // Serial.print("beginIdx: ");
            // Serial.println(beginIdx);
            // Serial.print("endIdx: ");
            // Serial.println(endIdx);
        #endif

        for (int i = beginIdx; i <= endIdx; i++)
        {
            leds[i] = CRGB(rOverride, gOverride, bOverride);
        }

        #if DEBUG == 1
            // Serial.println("SolidRing::iterateFrame - Done");
        #endif

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

};