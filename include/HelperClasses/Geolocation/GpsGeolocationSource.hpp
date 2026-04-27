#pragma once
#include "GeolocationInterface.hpp"
#include "TinyGPS++.h"

namespace NavigationModule
{
    class GpsGeolocationSource : public GeolocationInterface
    {
    public:
        GpsGeolocationSource(TinyGPSPlus& gps) : _gps(gps) {}

        bool TryGetCurrentLocation(double& outLat, double& outLon) override
        {
            if (!_gps.location.isValid())
            {
                ESP_LOGI(_TAG, "GPS location not valid");
                return false;
            }
            outLat = _gps.location.lat();
            outLon = _gps.location.lng();
            ESP_LOGI(_TAG, "Returning GPS location %.6f, %.6f", outLat, outLon);
            return true;
        }

    private:
        TinyGPSPlus& _gps;
        const char* _TAG = "GpsGeolocationSource";
    };
}


