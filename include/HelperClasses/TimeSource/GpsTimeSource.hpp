#pragma once
#include "TimeSourceInterface.hpp"
#include "TinyGPS++.h"
#include "NavigationUtils.h"

class GpsTimeSource : public SystemModule::TimeSourceInterface
{
public:
    GpsTimeSource(TinyGPSPlus& gps) : _gps(gps) {}

    bool TryGetCurrentUTC(time_t& outTime) override
    {
        if (!_gps.time.isValid() || !_gps.date.isValid() || _gps.date.value() == 0)
        {
            ESP_LOGI(_TAG, "Failed to get GPS time");
            return false;
        }
        outTime = NavigationUtils::PackedToTimeT(_gps.time.value(), _gps.date.value());
        ESP_LOGI(_TAG, "Returning timestamp %d from GPS", outTime);
        return true;
    }

private:
    TinyGPSPlus& _gps;
    const char * _TAG = "EzTimeSource";
};
