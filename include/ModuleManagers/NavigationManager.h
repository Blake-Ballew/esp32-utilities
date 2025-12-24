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
private:
    static constexpr const char *TAG = "NavigationManager";


public:
    NavigationManager() {}

    void InitializeUtils(CompassInterface *compass, Stream &gpsInputStream)
    {
        ESP_LOGI(TAG, "NavigationManager::InitializeUtils");
        NavigationUtils::Init(compass, gpsInputStream);

        NavigationUtils::SavedLocationsUpdated() += SaveLocationsToFlash;
        this->LoadLocationsFromFlash();

        StaticJsonDocument<128> calibrationData;
        auto returncode = FilesystemModule::Utilities::ReadFile(NavigationUtils::GetCalibrationFilename(), calibrationData);
        if (returncode == FilesystemModule::FilesystemReturnCode::FILESYSTEM_OK)
        {
            NavigationUtils::SetCalibrationData(calibrationData);

            std::string buf;
            serializeJson(calibrationData, buf);
            ESP_LOGI(TAG, "Calibration data loaded from flash: %s", buf.c_str());
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
            ESP_LOGE(TAG, "Failed to save locations to flash. Error code: %d", (int)returncode);
        }
        else
        {
            ESP_LOGI(TAG, "Saved locations to flash");
        }
    }

    void LoadLocationsFromFlash()
    {
        DynamicJsonDocument doc(16000);
        auto returncode = FilesystemModule::Utilities::ReadFile(LOCATION_FILE, doc);

        if (returncode != FilesystemModule::FilesystemReturnCode::FILESYSTEM_OK)
        {
            ESP_LOGE(TAG, "Failed to load locations from flash. Error code: %d", (int)returncode);
        }
        else
        {
            NavigationUtils::DeserializeSavedLocations(doc);
        }
    }
};