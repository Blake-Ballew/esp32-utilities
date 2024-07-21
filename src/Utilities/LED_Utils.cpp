#include "LED_Utils.h"

std::unordered_map<int, LED_Pattern_Status> LED_Utils::registeredPatterns;
int LED_Utils::patternTimerID = -1;
int LED_Utils::nextPatternID = 0;

int LED_Utils::registerPattern(LED_Pattern_Interface *pattern)
{
    registeredPatterns[nextPatternID] = {nextPatternID, pattern, 0};
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

    registeredPatterns[patternID].loopsRemaining = numLoops;

    if (numLoops != 0)
    {
        System_Utils::enableTimer(patternTimerID);
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
        System_Utils::setTimerInterval(patternTimerID, ms);
    }
}

bool LED_Utils::iteratePatterns()
{
    bool workToDo = false;

    for (auto &pattern : registeredPatterns)
    {
        if (pattern.second.loopsRemaining == 0)
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

    return workToDo;
}

void LED_Utils::iteratePattern(int patternID)
{
    if (registeredPatterns.find(patternID) == registeredPatterns.end())
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

void LED_Utils::setThemeColor(uint8_t r, uint8_t g, uint8_t b)
{
    LED_Pattern_Interface::setThemeColor(r, g, b);
}