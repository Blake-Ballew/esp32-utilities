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
    bool enabled;
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
    static void configurePattern(int patternID, JsonDocument &config);

    // Sets a pattern to loop n times, or indefinitely if numLoops is -1
    // Enables the pattern timer if it is not already running
    static void loopPattern(int patternID, int numLoops);

    // Enables a pattern
    static void enablePattern(int patternID);

    // Disables a pattern
    static void disablePattern(int patternID);

    // Clears the given pattern's LEDs and stops it from looping
    static void clearPattern(int patternID);

    // Sets the LED array to be used by the patterns
    static void setLeds(CRGB *leds, size_t numLeds);

    // Sets the tick rate for the LED patterns
    static void setTickRate(size_t ms);

    // Task to iterate patterns. Updates FastLED at the end
    // Returns false if there is no more work to do and the timer can be stopped
    // Otherwise, returns true
    static void iteratePatterns(void *pvParameters);

    // Function to iterate a single pattern once
    static void iteratePattern(int patternID);

    // Sets the user's theme color
    // This is used by patterns that don't have their own color configuration
    static void setThemeColor(uint8_t r, uint8_t g, uint8_t b);

    // Sets the InputID to LED index map
    static void setInputIdLedPins(std::unordered_map<uint8_t, uint8_t> inputIdLedPins);
    static std::unordered_map<uint8_t, uint8_t> &InputIdLedPins();

    static void SetIteratePatternTaskHandle(TaskHandle_t handle) { _IteratePatternsTaskHandle = handle; }
    
protected:
    static std::unordered_map<uint8_t, uint8_t> inputIdLedPins;

    static std::unordered_map<int, LED_Pattern_Status> registeredPatterns;
    static int nextPatternID;
    static size_t _PatternTickRateMS;
    static TaskHandle_t _IteratePatternsTaskHandle;
};