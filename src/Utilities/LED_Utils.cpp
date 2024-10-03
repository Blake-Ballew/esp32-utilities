#include "LED_Utils.h"

std::unordered_map<uint8_t, uint8_t> LED_Utils::inputIdLedPins;
CRGB LED_Utils::_ThemeColor;

std::unordered_map<int, LED_Pattern_Status> LED_Utils::registeredPatterns;
int LED_Utils::nextPatternID = 0;
size_t LED_Utils::_PatternTickRateMS = 50;

TaskHandle_t LED_Utils::_IteratePatternsTaskHandle = nullptr;

int LED_Utils::registerPattern(LED_Pattern_Interface *pattern)
{
    if (pattern == nullptr)
    {
        return -1;
    }

    registeredPatterns[nextPatternID] = {nextPatternID, pattern, 0, false};
    pattern->SetRegisteredPatternID(nextPatternID);
    return nextPatternID++;
}

void LED_Utils::unregisterPattern(int patternID)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    delete registeredPatterns[patternID].pattern;
    registeredPatterns.erase(patternID);
}

void LED_Utils::resetPattern(int patternID)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    registeredPatterns[patternID].pattern->resetPattern();
}

void LED_Utils::setAnimationLengthMS(int patternID, size_t ms)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    registeredPatterns[patternID].pattern->setAnimationLengthMS(ms);
}

void LED_Utils::setAnimationLengthTicks(int patternID, size_t ticks)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    registeredPatterns[patternID].pattern->setAnimationLengthTicks(ticks);
}

void LED_Utils::configurePattern(int patternID, JsonDocument &config)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    registeredPatterns[patternID].pattern->configurePattern(config);
}   

void LED_Utils::loopPattern(int patternID, int numLoops)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    registeredPatterns[patternID].loopsRemaining = numLoops;

    if (numLoops != 0)
        vTaskResume(_IteratePatternsTaskHandle);
}

void LED_Utils::enablePattern(int patternID)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    registeredPatterns[patternID].enabled = true;
}

void LED_Utils::disablePattern(int patternID)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    registeredPatterns[patternID].enabled = false;
    clearPattern(patternID);
}

void LED_Utils::clearPattern(int patternID)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    registeredPatterns[patternID].pattern->clearPattern();

    if (registeredPatterns[patternID].loopsRemaining != 0)
    {
        loopPattern(patternID, 0);
    }

    FastLED.show();
}

void LED_Utils::setLeds(CRGB *leds, size_t numLeds)
{
    LED_Pattern_Interface::setLeds(leds, numLeds);
}

void LED_Utils::setTickRate(size_t ms)
{
    LED_Pattern_Interface::setTickRate(ms);

    _PatternTickRateMS = ms;

    // if (patternTimerID != -1)
    // {
    //     System_Utils::changeTimerPeriod(patternTimerID, ms);
    // }
}

void LED_Utils::iteratePatterns(void *pvParameters)
{
    while (true)
    {
        bool workToDo = false;
        
        // Iterate all patterns that are enable with work to do
        for (auto &pattern : registeredPatterns)
        {
            taskYIELD();
            
            if (pattern.second.loopsRemaining == 0 || !pattern.second.enabled)
            {
                continue;
            }

            if (pattern.second.pattern->iterateFrame())
            {
                if (pattern.second.loopsRemaining > 0)
                {
                    pattern.second.loopsRemaining--;
                }
            }

            if (pattern.second.loopsRemaining != 0)
            {
                workToDo = true;
            }
        }

        // Check task notification for single iteration
        uint32_t notificationPatternID;
        auto notificationReady = xTaskNotifyWait(0, ULONG_MAX, &notificationPatternID, 0);
        int patternID = (int)notificationPatternID;

        if (notificationReady == pdTRUE && registeredPatterns.find(patternID) != registeredPatterns.end())
        {
            // Clear the notification
            xTaskNotifyStateClear(_IteratePatternsTaskHandle);
            
            #if DEBUG == 1
            // Serial.println("LED_Utils::iteratePatterns: Single iteration");
            #endif
            if (registeredPatterns[patternID].enabled)
            {
                registeredPatterns[patternID].pattern->iterateFrame();
            }
        }

        FastLED.show();
        
        if (!workToDo)
        {
            vTaskSuspend(_IteratePatternsTaskHandle);
        }

        vTaskDelay(pdMS_TO_TICKS(_PatternTickRateMS));
    }
}

void LED_Utils::iteratePattern(int patternID)
{
    #if DEBUG == 1
    // Serial.println("LED_Utils::iteratePattern");
    #endif
    if (_IteratePatternsTaskHandle != nullptr && patternID > -1)
    {
        vTaskResume(_IteratePatternsTaskHandle);
        uint32_t notificationPatternID = (uint32_t)patternID;
        xTaskNotify(_IteratePatternsTaskHandle, notificationPatternID, eSetValueWithOverwrite);
    }
}

void LED_Utils::setThemeColor(uint8_t r, uint8_t g, uint8_t b)
{
    _ThemeColor.r = r;
    _ThemeColor.g = g;
    _ThemeColor.b = b;
}

void LED_Utils:: setThemeColor(CRGB color)
{
    #if DEBUG == 1
        // Serial.print("LED_Utils::setThemeColor: ");
        // Serial.print(color.r);
        // Serial.print(", ");
        // Serial.print(color.g);
        // Serial.print(", ");
        // Serial.println(color.b);
    #endif
    _ThemeColor = color;
    LED_Pattern_Interface::SetThemeColor(_ThemeColor);
}

CRGB &LED_Utils::ThemeColor()
{
    return _ThemeColor;
}

void LED_Utils::setInputIdLedPins(std::unordered_map<uint8_t, uint8_t> inputIdLedPins)
{
    LED_Utils::inputIdLedPins = inputIdLedPins;
}

std::unordered_map<uint8_t, uint8_t> &LED_Utils::InputIdLedPins()
{
    return inputIdLedPins;
}