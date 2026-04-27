#pragma once

#include "CompassInterface.h"
#include "TinyGPS++.h"
#include "NavigationUtils.h"

namespace
{
    const char *LOCATION_FILE PROGMEM = "/SavedLocations.msgpk";
}

namespace NavigationModule
{
    class Manager
    {
    private:
        static constexpr const char* TAG = "NavigationManager";

    public:
        Manager() {}

        void InitializeUtils(CompassInterface* compass, Stream& gpsInputStream)
        {
            ESP_LOGI(TAG, "NavigationModule::Manager::InitializeUtils");
            NavigationModule::Utilities::Init(compass, gpsInputStream);

            NavigationModule::Utilities::SavedLocationsUpdated() += SaveLocationsToFlash;
            this->LoadLocationsFromFlash();

            StaticJsonDocument<256> calibrationData;
            auto returncode = FilesystemModule::Utilities::ReadFile(
                NavigationModule::Utilities::GetCalibrationFilename(), calibrationData);
            if (returncode == FilesystemModule::FilesystemReturnCode::FILESYSTEM_OK)
            {
                NavigationModule::Utilities::SetCalibrationData(calibrationData);

                std::string buf;
                serializeJson(calibrationData, buf);
                ESP_LOGI(TAG, "Calibration data loaded from flash: %s", buf.c_str());
            }
        }

        void InitializeUtils(CompassInterface* compass)
        {
            NavigationModule::Utilities::Init(compass);

            NavigationModule::Utilities::SavedLocationsUpdated() += SaveLocationsToFlash;
            this->LoadLocationsFromFlash();
        }

        // TODO: Implement if needed
        static void AutoStreamGPS()
        {
        }

        static void SaveLocationsToFlash()
        {
            DynamicJsonDocument doc(16000);

            NavigationModule::Utilities::SerializeSavedLocations(doc);
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
                NavigationModule::Utilities::DeserializeSavedLocations(doc);
            }
        }
    };
}

// Backward-compatibility alias — prefer NavigationModule::Manager in new code
using NavigationManager = NavigationModule::Manager;
