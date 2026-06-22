#pragma once

#include "LedPatternInterface.hpp"


// A pattern used to point a direction with the LED ring.
// The angle is degrees from the beginning index of the ring is supplied for the direction.
// The sharpness of the point is controlled by the fadeDegrees parameter.
// This can be used as a compass or a scrolling indicator in menus.
// This pattern is not animated and will be called when updates are needed.
class RingPoint : public LedPatternInterface
{
public:

    RingPoint(LedSegment segment)
        : LedPatternInterface(std::move(segment))
    {
        rOverride = 0;
        gOverride = 0;
        bOverride = 0;

        fadeDegrees = 0;
        directionDegrees = 0;

        setAnimationLengthTicks(1);
    }

    void configurePattern(JsonDocument &config)
    {
        if (!config["rOverride"].isNull())
        {
            rOverride = config["rOverride"];
        }

        if (!config["gOverride"].isNull())
        {
            gOverride = config["gOverride"];
        }

        if (!config["bOverride"].isNull())
        {
            bOverride = config["bOverride"];
        }

        if (!config["fadeDegrees"].isNull())
        {
            fadeDegrees = config["fadeDegrees"];
        }

        if (!config["directionDegrees"].isNull())
        {
            directionDegrees = config["directionDegrees"];
        }
    }

    bool iterateFrame()
    {
        ESP_LOGD(TAG, "RingPoint::iterateFrame");

        _segment.clear();

        CRGB outColor;
        if (rOverride == 0 && gOverride == 0 && bOverride == 0)
        {
            outColor = ThemeColor();
        }
        else
        {
            outColor = CRGB(rOverride, gOverride, bOverride);
        }

        for (size_t i = 0; i < _segment.length(); i++)
        {
            float brightness = _GetLEDBrightness(i);
            _segment[i] = CRGB(outColor.r * brightness, outColor.g * brightness, outColor.b * brightness);
        }

        return true;
    }

    void clearPattern()
    {
        LedPatternInterface::clearPattern();
        FastLED.show();
    }

    void SetRegisteredPatternID(int patternID) { _RegisteredPatternId() = patternID; }
    static int RegisteredPatternID() { return _RegisteredPatternId(); }

protected:
    static int &_RegisteredPatternId()
    {
        static int id = -1;
        return id;
    }

    uint8_t rOverride;
    uint8_t gOverride;
    uint8_t bOverride;

    float fadeDegrees;
    float directionDegrees;

    const char *TAG = "RingPoint";

    float _GetLEDBrightness(size_t segIdx)
    {
        float angle = (float)segIdx * 360.0f / (float)_segment.length();

        float angleDiff = abs(angle - directionDegrees);

        if (angleDiff > 180)
        {
            angleDiff = 360 - angleDiff;
        }

        if (angleDiff > fadeDegrees)
        {
            return 0;
        }

        return 1.0 - (angleDiff / fadeDegrees);
    }
};
