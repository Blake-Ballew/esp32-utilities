#pragma once

#include <FastLED.h>
#include "ButtonFlash.hpp"
#include "LED_Utils.h"
#include "DisplayUtilities.hpp"
#include <utility>
#include <vector>

#define LED_MS_PER_FRAME 50

#if HARDWARE_VERSION < 3
#define BUZZER_PIN 4
#else
#define BUZZER_PIN 7
#define HAPTIC_VIBRATION_PIN 6
#endif

// LED animation/rendering lives in the LedPatternInterface registry driven by
// LED_Utils (RingPoint, RingPulse, SolidRing, ScrollWheel, Flashlight, ...).
// LED_Manager is only responsible for bootstrapping FastLED + the pattern task
// and for the discrete hardware outputs (buzzer, haptic motor).
class LED_Manager
{
public:
    inline static CRGB *leds = nullptr;

    static void init(size_t numLeds, CRGB *ledBuffer, uint8_t cpuCore)
    {
        ESP_LOGI(TAG, "LED_Manager::init");
        leds = ledBuffer;

        ESP_LOGI(TAG, "Initializing FastLED with %d LEDs", numLeds);

        FastLED.setBrightness(255);
        FastLED.clear();
        FastLED.show();

        ESP_LOGI(TAG, "Starting LED pattern task on CPU core %d", cpuCore);

        int patternTaskID = System_Utils::registerTask(LED_Utils::iteratePatterns, "LED Task", 4096, NULL, 1, cpuCore);
        LED_Utils::SetIteratePatternTaskHandle(System_Utils::getTask(patternTaskID));

        LED_Utils::setTickRate(LED_MS_PER_FRAME);

        LedPatternInterface::SetThemeColor(LED_Utils::ThemeColor());
    }

    static void buzzerNotification(uint16_t frequency = 1000, size_t duration = 100)
    {
        tone(BUZZER_PIN, frequency, duration);
    }

    static void applyHapticFeedback(uint8_t intensity)
    {
#if HARDWARE_VERSION >= 3
        static constexpr TickType_t HAPTIC_MS = 80;

        // Constant drive for HAPTIC_MS, then off — same strong pulse as the old
        // blocking delay(80), but the turn-off is scheduled on a one-shot timer so
        // the caller (the render loop) keeps running instead of stalling 80 ms per
        // input. Re-firing mid-pulse just restarts the window from the latest input.
        analogWrite(HAPTIC_VIBRATION_PIN, intensity);

        if (hapticTimer == nullptr)
        {
            hapticTimer = xTimerCreateStatic(
                "haptic", HAPTIC_MS / portTICK_PERIOD_MS, pdFALSE, nullptr,
                hapticOff, &hapticTimerBuffer);
        }
        xTimerReset(hapticTimer, 0);
#endif
    }

private:
    // One-shot timer that switches the haptic motor off, so applyHapticFeedback()
    // never blocks the calling task (e.g. the display/render loop).
    inline static TimerHandle_t hapticTimer = nullptr;
    inline static StaticTimer_t hapticTimerBuffer;

    // Timer callback: runs in the FreeRTOS timer service (Tmr Svc) daemon, not the
    // caller's task, so turning the motor off costs the display/render loop nothing.
    static void hapticOff(TimerHandle_t)
    {
#if HARDWARE_VERSION >= 3
        analogWrite(HAPTIC_VIBRATION_PIN, 0);
#endif
    }
};
