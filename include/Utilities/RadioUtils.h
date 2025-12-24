#pragma once

#include "System_Utils.h"
#include "ConnectivityUtils.h"
#include "WiFi.h"
#include <esp_now.h>
#include "esp_smartconfig.h"
#include "esp_wifi.h"

namespace ConnectivityModule
{

    enum WiFiRadioState
    {
        RADIO_STATE_OFF = 0,
        RADIO_STATE_STA = 1,
        RADIO_STATE_AP = 2,
        RADIO_STATE_ESP_NOW = 3,
        RADIO_STATE_BT = 4
    };

    class RadioUtils
    {
    public:
        static WiFiRadioState &RadioState()
        {
            static WiFiRadioState radioState = RADIO_STATE_OFF;
            return radioState;
        }

        static void EnableRadio()
        {
            WiFi.setSleep(false);
        }

        static void DisableRadio()
        {
            WiFi.setSleep(true);
            RadioState() = RADIO_STATE_OFF;
        }

        static bool IsRadioActive()
        {
            return !WiFi.getSleep();
        }

        static int RadioChannel()
        {
            return WiFi.channel();
        }

        static uint8_t &EspNowChanel()
        {
            static uint8_t channel = 1;
            return channel;
        }

        static void SetRadioChannel(uint8_t channel)
        {
            esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        }

        static void SetRadioChannel()
        {
            SetRadioChannel(EspNowChanel());
        }

        static void InitializeEspNow()
        {
            WiFi.mode(WIFI_STA);
            WiFi.disconnect();
            EnableRadio();
            SetRadioChannel(EspNowChanel());

            if (esp_now_init() != ESP_OK)
            {
                DisableRadio();
                ESP_LOGE(TAG, "esp_now_init failed");
                return;
            }

            RadioState() = RADIO_STATE_ESP_NOW;
            // TODO: re-add saved peers as they get deleted when esp-now is disabled.
        }

        static void DeinitializeEspNow()
        {
            esp_now_deinit();
            DisableRadio();
            WiFi.disconnect();
            WiFi.mode(WIFI_OFF);
        }

        static void InitializeSmartConfig()
        {
            EnableRadio();
            WiFi.disconnect();
            WiFi.mode(WIFI_STA);
            WiFi.beginSmartConfig();
        }

        // STA and SmartConfig
        static bool CheckSmartConfig()
        {
            auto result = WiFi.smartConfigDone();
            if (result)
            {
                RadioState() = RADIO_STATE_STA;
                return true;
            }

            return false;
        }

        static void DeinitializeSmartConfig()
        {
            WiFi.stopSmartConfig();
        }

        static bool IsWiFiActive()
        {
            return WiFi.status() == WL_CONNECTED;
        }

        // Connects to AP using saved password from last SmartConfig connection
        static bool ConnectToAccessPoint()
        {
            EnableRadio();
            WiFi.disconnect();
            WiFi.mode(WIFI_STA);
            auto reuslt = WiFi.begin() == WL_CONNECTED;
            if (reuslt)
            {
                RadioState() = RADIO_STATE_STA;
            }
            return reuslt;
        }

        static bool ConnectToAccessPoint(std::string ssid, std::string password)
        {
            EnableRadio();
            WiFi.disconnect();
            WiFi.mode(WIFI_STA);
            auto reuslt = WiFi.begin(ssid.c_str(), password.c_str()) == WL_CONNECTED;
            if (reuslt)
            {
                RadioState() = RADIO_STATE_STA;
            }
            return reuslt;
        }

        // AP
        static std::string ApSSID()
        {
            static std::string _ApSSID = "ESP32-Utilities-AP";
            return _ApSSID;
        }

        static std::string ApPassword()
        {
            static std::string _ApPassword = "esp-ap-password";
            return _ApPassword;
        }

        static bool StartAccessPoint()
        {
            WiFi.mode(WIFI_AP);
            auto result = WiFi.softAP(ApSSID().c_str(), ApPassword().c_str());
            if (result)
            {
                RadioState() = RADIO_STATE_AP;
            }
            return result;
        }

        static void StopAccessPoint()
        {
            WiFi.softAPdisconnect();
            DisableRadio();
            WiFi.mode(WIFI_OFF);
            RadioState() = RADIO_STATE_OFF;       
        }
    };
}