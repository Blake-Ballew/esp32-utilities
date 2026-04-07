#include "LedPatternInterface.hpp"
CRGB black = CRGB(0, 0, 0);

size_t LedPatternInterface::msPerTick = 15;

CRGB &LedPatternInterface::themeColor = black;