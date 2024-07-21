#pragma once

#include "ArduinoJson.h"
#include "System_Utils.h"
#include "FastLED.h"
#include "LED_Pattern_Interface.h"
#include <unordered_map>

// Struct to hold pattern objects and their status
struct LED_Pattern_Status
{
    int animationID;
    LED_Pattern_Interface *pattern;
    int loopsRemaining;
};

class LED_Utils
{
public:
    // Registers a pattern and returns the pattern ID
    static int registerPattern(LED_Pattern_Interface *pattern);

    // Unregisters a pattern and deletes it
    static void unregisterPattern(int patternID);

    // Resets a pattern to its initial state
    static void resetPattern(int patternID);

    // Sets a pattern's animation length in ms
    static void setAnimationLengthMS(int patternID, size_t ms);

    // Sets a pattern's animation length in ticks
    static void setAnimationLengthTicks(int patternID, size_t ticks);

    // Sends a configuration object to a pattern
    static void configurePattern(int patternID, JsonObject &config);

    // Sets a pattern to loop n times, or indefinitely if numLoops is -1
    // Enables the pattern timer if it is not already running
    static void loopPattern(int patternID, int numLoops);

    // Sets the LED array to be used by the patterns
    static void setLeds(CRGB *leds, size_t numLeds);

    // Sets the tick rate for the LED patterns
    static void setTickRate(size_t ms);

    // Timer function to iterate patterns. Updates FastLED at the end
    // Returns false if there is no more work to do and the timer can be stopped
    // Otherwise, returns true
    static bool iteratePatterns();

    // Function to iterate a single pattern once
    static void iteratePattern(int patternID);

    // Sets the patternTimerID
    static void setPatternTimerID(int timerID);

    // Sets the user's theme color
    // This is used by patterns that don't have their own color configuration
    static void setThemeColor(uint8_t r, uint8_t g, uint8_t b);
    
protected:
    static std::unordered_map<int, LED_Pattern_Status> registeredPatterns;
    static int patternTimerID;
    static int nextPatternID;
};