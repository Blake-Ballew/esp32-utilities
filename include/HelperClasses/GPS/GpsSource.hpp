#pragma once

#include "GeolocationInterface.hpp"
#include "TimeSourceInterface.hpp"
#include "TinyGPS++.h"
#include "NavigationUtils.h"

namespace NavigationModule
{
    class GpsSource : public SystemModule::TimeSourceInterface, public GeolocationInterface
    {
    public:
        GpsSource(TinyGPSPlus& gps, Stream& gpsStream) : _gps(gps), _gpsStream(gpsStream) {}

        bool TryGetCurrentUTC(time_t& outTime) override
        {
            _UpdateGps();
            
            if (!_gps.time.isValid() || !_gps.date.isValid() || _gps.date.value() == 0)
            {
                ESP_LOGI(_TAG, "Failed to get GPS time");
                return false;
            }
            outTime = NavigationUtils::PackedToTimeT(_gps.time.value(), _gps.date.value());
            ESP_LOGI(_TAG, "Returning timestamp %d from GPS", outTime);
            return true;
        }

        bool TryGetCurrentLocation(double& outLat, double& outLon) override
        {
            _UpdateGps();

            // TODO: check how old this location is before returning it
            if (!_lastLocation.isValid() || (_lastLocation.lat() == 0 && _lastLocation.lng() == 0))
            {
                ESP_LOGI(_TAG, "GPS location not valid");
                return false;
            }
            outLat = _lastLocation.lat();
            outLon = _lastLocation.lng();
            ESP_LOGI(_TAG, "Returning GPS location %.6f, %.6f", outLat, outLon);
            return true;
        }

    protected:
        TinyGPSPlus& _gps;
        Stream &_gpsStream;
        const char * _TAG = "GpsSource";
        TinyGPSLocation _lastLocation;

        void _UpdateGps()
        {
            ESP_LOGI(_TAG, "Reading %d bytes from GPS stream", _gpsStream.available());
            std::string debugStr;

            while (_gpsStream.available() > 0)
            {
                char c = _gpsStream.read();
                debugStr += c;
                _gps.encode(c);
            }
            if (!debugStr.empty())
            {
                ESP_LOGI(_TAG, "GPS data received: %s", debugStr.c_str());
            }

            ESP_LOGI(_TAG, "Num GPS satellites: %d, HDOP: %.1f", _gps.satellites.value(), _gps.hdop.hdop());

            if (_gps.location.isValid())
            {
                _lastLocation = _gps.location;
            }
        }
    };
}