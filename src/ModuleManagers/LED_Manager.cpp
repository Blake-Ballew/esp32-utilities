#include "LED_Manager.h"

CRGB *LED_Manager::leds = nullptr;
bool LED_Manager::flashlightOn = false;

uint8_t LED_Manager::r = 0;
uint8_t LED_Manager::g = 0;
uint8_t LED_Manager::b = 255;

int LED_Manager::buttonFlashPatternID = -1;

int LED_Manager::patternTaskID = -1;

void LED_Manager::init(size_t numLeds, uint8_t cpuCore)
{
    leds = new CRGB[numLeds];
    LED_Utils::setLeds(leds, numLeds);

    FastLED.addLeds<LED_TYPE, LED_PIN, LED_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(255);
    FastLED.clear();
    FastLED.show();

    patternTaskID = System_Utils::registerTask(LED_Utils::iteratePatterns, "LED Task", 4096, NULL, 5, cpuCore);
    LED_Utils::SetIteratePatternTaskHandle(System_Utils::getTask(patternTaskID));

    LED_Utils::setTickRate(LED_MS_PER_FRAME);

    // patternTimer = xTimerCreateStatic("Pattern Timer", 17, true, NULL, updatePattern, &patternTimerBuffer);
}

void LED_Manager::InitializeInputIdLedPins(std::unordered_map<uint8_t, uint8_t> inputIDLedIdx)
{
    LED_Utils::setInputIdLedPins(inputIDLedIdx);
}

void LED_Manager::initializeButtonFlashAnimation()
{
    auto inputIDLedIdx = LED_Utils::InputIdLedPins();
    auto btnFlash = new Button_Flash(inputIDLedIdx);

    buttonFlashPatternID = LED_Utils::registerPattern(btnFlash);
    LED_Utils::enablePattern(buttonFlashPatternID);
    LED_Utils::setAnimationLengthMS(buttonFlashPatternID, 300);
    Display_Utils::getInputRaised() += inputButtonFlash;
}

void LED_Manager::inputButtonFlash(uint8_t inputID)
{
    #if DEBUG == 1
    Serial.print("Button flash input: ");
    Serial.println(inputID);
    #endif
    StaticJsonDocument<64> cfg;
    cfg["inputID"] = inputID;

    LED_Utils::configurePattern(buttonFlashPatternID, cfg);
    LED_Utils::loopPattern(buttonFlashPatternID, 1);
}

void LED_Manager::pointNorth(int Azimuth)
{
    interpolateLEDsDegrees(Azimuth, 500.0f, r, g, b);

    /*int ledIndex = (Azimuth / 360.0) * NUM_COMPASS_LEDS;
    for (uint8_t i = 0; i < NUM_COMPASS_LEDS; i++)
    {
        leds[i] = CRGB::Black;
        if (i == ledIndex)
        {
            leds[i] = CRGB(r, g, b);
        }
    }
    FastLED.show();*/
}

void LED_Manager::pointToHeading(int Azimuth, double heading, double distanceAway, uint8_t r, uint8_t g, uint8_t b)
{
#if DEBUG == 1
    static uint8_t refreshTicks = 0;

    if (!refreshTicks)
    {
        Serial.print("Azimuth: ");
        Serial.println(Azimuth);
        Serial.print("Heading: ");
        Serial.println(heading);
        Serial.print("Distance: ");
        Serial.println(distanceAway);
    }
    refreshTicks++;
#endif
    double deg = heading - Azimuth;
    if (deg < 0)
    {
        deg += 360.0f;
    }

    interpolateLEDsDegrees(deg, distanceAway, r, g, b);
}

void LED_Manager::lightRing(uint8_t r, uint8_t g, uint8_t b)
{
    for (uint8_t i = 0; i < NUM_COMPASS_LEDS; i++)
    {
        leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
}

void LED_Manager::clearRing()
{
    for (uint8_t i = 0; i < NUM_COMPASS_LEDS; i++)
    {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
}

void LED_Manager::toggleFlashlight()
{
    if (flashlightOn)
    {
        for (uint8_t i = 23; i < 30; i++)
        {
            leds[i] = CRGB::Black;
        }
        FastLED.show();
        flashlightOn = false;
    }
    else
    {
        for (uint8_t i = 23; i < 30; i++)
        {
            leds[i] = CRGB::White;
        }
        FastLED.show();
        flashlightOn = true;
    }
}

void LED_Manager::buzzerNotification(uint16_t frequency, size_t duration)
{
    tone(BUZZER_PIN, frequency, duration);
}

void LED_Manager::ledShutdownAnimation()
{
    float brightness = 1.0f;
    while (brightness >= 0.0f)
    {
        for (uint8_t i = 0; i < NUM_COMPASS_LEDS; i++)
        {
            leds[i] = CRGB(255 * brightness, 0, 0);
        }
        FastLED.show();
        brightness -= 0.01f;
        delay(50);
    }
}

// void LED_Manager::pulseButton(uint8_t buttonNumber)
// {
//     size_t ledIdx;

//     if (Button_Flash::ticksRemaining > 0)
//     {
//         return;
//     }

//     switch (buttonNumber)
//     {
//     case BUTTON_1:
//         ledIdx = 22;
//         break;
//     case BUTTON_2:
//         ledIdx = 19;
//         break;
//     case BUTTON_3:
//         ledIdx = 18;
//         break;
//     case BUTTON_4:
//         ledIdx = 17;
//         break;
//     case BUTTON_SOS: // SOS
//         ledIdx = 16;
//         break;
//     case ENC_UP: // Encoder up
//         ledIdx = 20;
//         break;
//     case ENC_DOWN: // Encoder down
//         ledIdx = 21;
//         break;
//     default:
//         return;
//     }

//     Button_Flash::init(ledIdx, r, g, b);
//     xTimerStart(patternTimer, 0);
// }

void LED_Manager::pulseCircle(uint8_t r, uint8_t g, uint8_t b, size_t tick)
{
    float brightness = ((sin(tick / 10.0) + 1) / 2);
    for (uint8_t i = 0; i < NUM_COMPASS_LEDS; i++)
    {
        leds[i] = CRGB(r * brightness, g * brightness, b * brightness);
    }
    FastLED.show();
}

void LED_Manager::displayScrollWheel(size_t currentIdx, size_t listSize)
{
    interpolateLEDsDegrees((currentIdx / (float)listSize) * 360.0f, 500.0f, r, g, b);
}

void LED_Manager::interpolateLEDsDegrees(double deg, double distanceAway, uint8_t r, uint8_t g, uint8_t b)
{
    if (distanceAway > 500.0f)
    {
        distanceAway = 500.0f;
    }
    else if (distanceAway < 20.0f)
    {
        distanceAway = 20.0f;
    }

    // LEDs should form a fine point when distance is greater
    // LEDs should light up to a quarter of the circle when distance is less

    double ledDirection = (deg / 360.0f) * NUM_COMPASS_LEDS;
    float distanceMultiplier = (7.0f / 2000.0f) * (distanceAway - 20.0f) + (1.0f / 8.0f);

#if DEBUG == 1
    static uint8_t refreshTicks = 0;

    refreshTicks += 8;
    if (!refreshTicks)
    {
        Serial.print("Distance: ");
        Serial.println(distanceAway);
        // Serial.print("Degrees: ");
        // Serial.println(deg);
        // Serial.print("Distance multiplier: ");
        // Serial.println(distanceMultiplier);
        // Serial.println("LED brightness: ");
    }

#endif

    for (int i = 0; i < NUM_COMPASS_LEDS; i++)
    {

        // Because circle math is linear and circles are not, we have to calculate 3 brightness values for the beginning and end  of the circle to look right
        float brightness = -1.0f * distanceMultiplier * (i - ledDirection) * (i - ledDirection) + 1.0f;
        float brightness2 = -1.0f * distanceMultiplier * (i - (ledDirection - NUM_COMPASS_LEDS)) * (i - (ledDirection - NUM_COMPASS_LEDS)) + 1.0f;
        float brightness3 = -1.0f * distanceMultiplier * (i - (ledDirection + NUM_COMPASS_LEDS)) * (i - (ledDirection + NUM_COMPASS_LEDS)) + 1.0f;

#if DEBUG == 1
        if (!refreshTicks)
        {

            // Serial.print("LED ");
            // Serial.print(i);
            // Serial.print(": B1: ");
            // Serial.print(brightness);
            // Serial.print(" B2: ");
            // Serial.print(brightness2);
            // Serial.print(" B3: ");
            // Serial.println(brightness3);
        }

#endif
        brightness = max(brightness, brightness2);
        brightness = max(brightness, brightness3);

        if (brightness < 0.0f)
        {
            brightness = 0.0f;
        }
        else if (brightness > 1.0f)
        {
            brightness = 1.0f;
        }

        leds[i] = CRGB(r * brightness, g * brightness, b * brightness);
    }
    FastLED.show();
}

// void LED_Manager::updatePattern(TimerHandle_t xTimer)
// {
//     Button_Flash::update();

//     if (Button_Flash::ticksRemaining == 0)
//     {
//         xTimerStop(patternTimer, 0);
//     }
// }