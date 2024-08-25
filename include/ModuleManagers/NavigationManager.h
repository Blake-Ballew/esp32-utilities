#pragma once

#include "CompassInterface.h"
#include "TinyGPS++.h"
#include "NavigationUtils.h"

namespace
{
    const char *LOCATION_FILE PROGMEM = "/SavedLocations.msgpk";
}

// Manager class to iniitialize NavigationUtils
class NavigationManager
{
public:
    NavigationManager() {}

    void InitializeUtils(CompassInterface *compass, Stream &gpsInputStream)
    {
        NavigationUtils::Init(compass, gpsInputStream);

        NavigationUtils::SavedLocationsUpdated() += SaveLocationsToFlash;
        this->LoadLocationsFromFlash();
    }

    void InitializeUtils(CompassInterface *compass)
    {
        NavigationUtils::Init(compass);

        NavigationUtils::SavedLocationsUpdated() += SaveLocationsToFlash;
        this->LoadLocationsFromFlash();
    }

    // TODO: Implement if needed
    static void AutoStreamGPS()
    {

    }

    static void SaveLocationsToFlash()
    {
        DynamicJsonDocument doc(1024);

        NavigationUtils::SerializeSavedLocations(doc);
        auto returncode = FilesystemUtils::WriteFile(LOCATION_FILE, doc);

        if (returncode != FilesystemReturnCode::FILESYSTEM_OK)
        {
            #if DEBUG == 1
            Serial.print("Failed to save locations to flash. Error code: ");
            Serial.println((int)returncode);
            #endif
        }
    }

    void LoadLocationsFromFlash()
    {
        DynamicJsonDocument doc(1024);
        auto returncode = FilesystemUtils::ReadFile(LOCATION_FILE, doc);

        if (returncode != FilesystemReturnCode::FILESYSTEM_OK)
        {
            #if DEBUG == 1
            Serial.print("Failed to load locations from flash. Error code: ");
            Serial.println((int)returncode);
            #endif
        }
        else
        {
            NavigationUtils::DeserializeSavedLocations(doc);
        }
    }
};