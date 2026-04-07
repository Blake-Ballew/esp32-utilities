#pragma once

#include "FastLED.h"
#include "ArduinoJson.h"
#include "LedSegment.hpp"

class LedPatternInterface
{
public:
    LedPatternInterface() {
        currTick = 0;
        animationTicks = 0;
        animationMS = 0;
        startTime = 0;
    }

    LedPatternInterface(LedSegment segment)
        : _segment(std::move(segment))
    {
        currTick = 0;
        animationTicks = 0;
        animationMS = 0;
        startTime = 0;
    }

    // Used to pass custom parameters to the pattern
    // This function will typically follow the pattern of:
    // if (config.containsKey("key")) { key = config["key"]; }
    virtual void configurePattern(JsonDocument &config) {}

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

    static void setTickRate(size_t ms) {
        msPerTick = ms;
    }

    // Called to iterate a single frame of the animation
    // Some animations only have one frame and will only be called here
    // Returns true if the last frame of the loop has played
    virtual bool iterateFrame() { return true; }

    // Clears the segment and resets animation state
    virtual void clearPattern() {
        _segment.clear();
        resetPattern();
    }

    // Every pattern should have a static member holding the registered pattern id
    // This is so different modules can use the same pattern without re-registering it
    virtual void SetRegisteredPatternID(int patternID) = 0;

    static void SetThemeColor(CRGB &color) {
        themeColor = color;
    }

    static CRGB &ThemeColor() { return themeColor; }

protected:
    LedSegment _segment;

    static size_t msPerTick;
    static CRGB &themeColor;

    // Length of an animation loop
    size_t animationTicks;
    size_t animationMS;

    // Current progress of animation
    size_t currTick;

    // Timestamp an animation was started
    size_t startTime;

};
