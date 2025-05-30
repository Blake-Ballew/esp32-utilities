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
        #if DEBUG == 1
        Serial.println("NavigationManager::InitializeUtils");
        #endif
        NavigationUtils::Init(compass, gpsInputStream);

        NavigationUtils::SavedLocationsUpdated() += SaveLocationsToFlash;
        this->LoadLocationsFromFlash();

        StaticJsonDocument<128> calibrationData;
        auto returncode = FilesystemModule::Utilities::ReadFile(NavigationUtils::GetCalibrationFilename(), calibrationData);
        if (returncode == FilesystemModule::FilesystemReturnCode::FILESYSTEM_OK)
        {
            NavigationUtils::SetCalibrationData(calibrationData);

            #if DEBUG == 1
            Serial.println("Calibration data loaded from flash.");
            serializeJson(calibrationData, Serial);
            Serial.println();
            #endif
        }
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
        DynamicJsonDocument doc(16000);

        NavigationUtils::SerializeSavedLocations(doc);
        auto returncode = FilesystemModule::Utilities::WriteFile(LOCATION_FILE, doc);

        if (returncode != FilesystemModule::FilesystemReturnCode::FILESYSTEM_OK)
        {
            #if DEBUG == 1
            Serial.print("Failed to save locations to flash. Error code: ");
            Serial.println((int)returncode);
            #endif
        }
        else
        {
            #if DEBUG == 1
            Serial.println("Saved locations to flash.");
            #endif
        }
    }

    void LoadLocationsFromFlash()
    {
        DynamicJsonDocument doc(16000);
        auto returncode = FilesystemModule::Utilities::ReadFile(LOCATION_FILE, doc);

        if (returncode != FilesystemModule::FilesystemReturnCode::FILESYSTEM_OK)
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