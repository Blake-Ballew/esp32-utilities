#include "LED_Utils.h"

std::unordered_map<uint8_t, uint8_t> LED_Utils::inputIdLedPins;

std::unordered_map<int, LED_Pattern_Status> LED_Utils::registeredPatterns;
int LED_Utils::patternTimerID = -1;
int LED_Utils::nextPatternID = 0;

int LED_Utils::registerPattern(LED_Pattern_Interface *pattern)
{
    if (pattern == nullptr)
    {
        return -1;
    }

    registeredPatterns[nextPatternID] = {nextPatternID, pattern, 0};
    pattern->setRegisteredPatternID(nextPatternID);
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

void LED_Utils::configurePattern(int patternID, JsonObject &config)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    registeredPatterns[patternID].pattern->configurePattern(config);
}   

void LED_Utils::loopPattern(int patternID, int numLoops)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end() || patternTimerID == -1)
    {
        return;
    }

    // Ensure pattern is enabled
    if (!registeredPatterns[patternID].enabled)
    {
        return;
    }

    registeredPatterns[patternID].loopsRemaining = numLoops;

    if (numLoops != 0 && !System_Utils::isTimerActive(patternTimerID))
    {
        startPatternTimer();
    }
    else if (numLoops == 0)
    {
        for (auto &pattern : registeredPatterns)
        {
            if (pattern.second.loopsRemaining != 0)
            {
                return;
            }
        }

        stopPatternTimer();
    }
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
}

void LED_Utils::setLeds(CRGB *leds, size_t numLeds)
{
    LED_Pattern_Interface::setLeds(leds, numLeds);
}

void LED_Utils::setTickRate(size_t ms)
{
    LED_Pattern_Interface::setTickRate(ms);

    if (patternTimerID != -1)
    {
        System_Utils::changeTimerPeriod(patternTimerID, ms);
    }
}

void LED_Utils::iteratePatterns()
{
    bool workToDo = false;

    for (auto &pattern : registeredPatterns)
    {
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

            if (pattern.second.loopsRemaining != 0)
            {
                workToDo = true;
            }
        }
    }

    FastLED.show();

    if (!workToDo)
    {
        stopPatternTimer();
    }
}

void LED_Utils::iteratePattern(int patternID)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
    {
        return;
    }

    if (!registeredPatterns[patternID].enabled)
    {
        return;
    }

    registeredPatterns[patternID].pattern->iterateFrame();
    FastLED.show();
}

void LED_Utils::setPatternTimerID(int timerID)
{
    patternTimerID = timerID;
}

void LED_Utils::startPatternTimer()
{
    System_Utils::startTimer(patternTimerID);
}

void LED_Utils::stopPatternTimer()
{
    System_Utils::stopTimer(patternTimerID);
}

void LED_Utils::setThemeColor(uint8_t r, uint8_t g, uint8_t b)
{
    LED_Pattern_Interface::setThemeColor(r, g, b);
}

void LED_Utils::setInputIdLedPins(std::unordered_map<uint8_t, uint8_t> inputIdLedPins)
{
    LED_Utils::inputIdLedPins = inputIdLedPins;
}

std::unordered_map<uint8_t, uint8_t> &LED_Utils::InputIdLedPins()
{
    return inputIdLedPins;
}