#pragma once

#include "globalDefines.h"
#include "FastLED.h"
#include "ArduinoJson.h"

class LED_Pattern_Interface
{
public:
    LED_Pattern_Interface() {
        currTick = 0;
        animationTicks = 0;
        animationMS = 0;
        startTime = 0;
    }

    // Used to pass custom parameters to the pattern
    // This function will typically follow the pattern of:
    // if (config.containsKey("key")) { key = config["key"]; }
    virtual void configurePattern(JsonObject &config) {}

    // Called to reset the pattern to its initial state
    void resetPattern() {
        currTick = 0;
        startTime = 0;
    }

    void setAnimationLengthMS(size_t ms) {
        animationTicks = ms / msPerTick;
        animationMS = ms;
    }

    void setAnimationLengthTicks(size_t ticks) {
        animationTicks = ticks;
        animationMS = ticks * msPerTick;
    }

    static void setLeds(CRGB *leds, size_t numLeds) {
        LED_Pattern_Interface::leds = leds;
        LED_Pattern_Interface::numLeds = numLeds;
    }

    static void setTickRate(size_t ms) {
        msPerTick = ms;
    }

    // Called to iterate a single frame of the animation
    // Some animations only have one frame and will only be called here
    // Returns true if the last frame of the loop has played
    virtual bool iterateFrame() { return true; }

    // Clears LEDs and resets the pattern
    virtual void clearPattern() {}

    // Every pattern should have a static member holding the registered pattern id
    // This is so different modules can use the same pattern without re-registering it
    virtual void SetRegisteredPatternID(int patternID) = 0;

    static void setThemeColor(uint8_t r, uint8_t g, uint8_t b) {
        r = r;
        g = g;
        b = b;
    }

protected:
    static CRGB *leds;
    static size_t numLeds;
    static size_t msPerTick;
    static uint8_t r;
    static uint8_t g;
    static uint8_t b;

    // Length of an animation loop
    size_t animationTicks;
    size_t animationMS;

    // Current progress of animation
    size_t currTick;
    
    // Timestamp an animation was started
    size_t startTime;

};