#pragma once
#include <Arduino.h>

namespace NavigationModule
{
    class GeolocationInterface
    {
    public:
        // Returns true and populates outLat/outLon (WGS84 decimal degrees) on success
        virtual bool TryGetCurrentLocation(double& outLat, double& outLon) = 0;
        virtual ~GeolocationInterface() = default;
    };
}
