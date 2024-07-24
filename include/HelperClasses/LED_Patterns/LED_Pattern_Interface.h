#pragma once

#include "globalDefines.h"
#include "FastLED.h"
#include "ArduinoJson.h"

class LED_Pattern_Interface
{
public:
    // Used to pass custom parameters to the pattern
    // This function will typically follow the pattern of:
    // if (config.containsKey("key")) { key = config["key"]; }
    virtual void configurePattern(JsonObject &config) {}

    // Called to reset the pattern to its initial state
    void resetPattern() {
        currTick = 0;
    }

    void setAnimationLengthMS(size_t ms) {
        animationTicks = ms / msPerTick;
    }

    void setAnimationLengthTicks(size_t ticks) {
        animationTicks = ticks;
    }

    static void setLeds(CRGB *leds, size_t numLeds) {
        leds = leds;
        numLeds = numLeds;
    }

    static void setTickRate(size_t ms) {
        msPerTick = ms;
    }

    // Called to iterate a single frame of the animation
    // Some animations only have one frame and will only be called here
    // Returns true if the last frame of the loop has played
    virtual bool iterateFrame() { return true; }

    // Every pattern should have a static member holding the registered pattern id
    // This is so different modules can use the same pattern without re-registering it
    virtual void setRegisteredPatternID(int patternID) = 0;
    virtual int registeredPatternID() = 0;

    static void setThemeColor(uint8_t r, uint8_t g, uint8_t b) {
        r = r;
        g = g;
        b = b;
    }

protected:
    static inline CRGB *leds = nullptr;
    static inline size_t numLeds = 0;
    static inline size_t msPerTick = 15;
    static inline uint8_t r = 0;
    static inline uint8_t g = 0;
    static inline uint8_t b = 255;

    // Total ticks in an animation loop
    size_t animationTicks;

    // Current tick in the animation loop
    size_t currTick;

};