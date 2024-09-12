#include "LED_Pattern_Interface.h"
CRGB black = CRGB(0, 0, 0);

CRGB *LED_Pattern_Interface::leds = nullptr;
size_t LED_Pattern_Interface::numLeds = 0;
size_t LED_Pattern_Interface::msPerTick = 15;

CRGB &LED_Pattern_Interface::themeColor = black;