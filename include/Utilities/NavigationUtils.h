#pragma once

#include "System_Utils.h"
#include "CompassInterface.h"
#include "TinyGPS++.h"
#include <string>

struct SavedLocation
{
    std::string Name;
    double Latitude;
    double Longitude;
};

// Static class to manage navigation functions including the compass and the GPS.
class NavigationUtils
{
public:
    static void Init(CompassInterface *compass);
    static void Init(CompassInterface *compass, Stream &gpsInputStream);

    // GPS Functionality
    static void UpdateGPS();
    static bool IsGPSConnected();
    static TinyGPSLocation GetLocation();
    static TinyGPSTime GetTime();
    static TinyGPSDate GetDate();
    static uint64_t GetTimeDifference(uint32_t time1, uint32_t date1, uint32_t time2, uint32_t date2);
    static uint64_t GetTimeDifference(uint32_t time1, uint32_t date1);

    // Compass Functionality
    static int GetAzimuth();
    static double GetDistanceTo(double lat, double lon);
    static double GetHeadingTo(double lat, double lon);
    static int GetX();
    static int GetY();
    static int GetZ();

    // Saved Locations
    static EventHandler &SavedLocationsUpdated() { return _SavedLocationsUpdated; }

    static void AddSavedLocation(SavedLocation location);
    static void RemoveSavedLocation(std::vector<SavedLocation>::iterator &locationIt);
    static void UpdateSavedLocation(std::vector<SavedLocation>::iterator &locationIt, SavedLocation location);
    static std::vector<SavedLocation>::iterator GetSavedLocationsBegin() { return _SavedLocations.begin(); }
    static std::vector<SavedLocation>::iterator GetSavedLocationsEnd() { return _SavedLocations.end(); }
    static size_t GetSavedLocationsSize() { return _SavedLocations.size(); }

    static void SerializeSavedLocations(JsonDocument &doc);
    static void DeserializeSavedLocations(JsonDocument &doc);

    static void FlashSampleLocations();

protected:
    static CompassInterface *_Compass;
    static TinyGPSPlus _GPS;
    static Stream &_GpsInputStream;

    static TinyGPSLocation _LastCoordinate;

    static EventHandler _SavedLocationsUpdated;

    static std::vector<SavedLocation> _SavedLocations;
};