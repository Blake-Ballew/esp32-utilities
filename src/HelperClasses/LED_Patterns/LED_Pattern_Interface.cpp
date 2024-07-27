#include "LED_Pattern_Interface.h"

CRGB *LED_Pattern_Interface::leds = nullptr;
size_t LED_Pattern_Interface::numLeds = 0;
size_t LED_Pattern_Interface::msPerTick = 15;
uint8_t LED_Pattern_Interface::r = 0;
uint8_t LED_Pattern_Interface::g = 0;
uint8_t LED_Pattern_Interface::b = 255;