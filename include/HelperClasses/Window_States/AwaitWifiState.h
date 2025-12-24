#pragma once

#include "Window_State.h"
#include "RadioUtils.h"
#include "ConnectivityUtils.h"

class AwaitWifiState : public Window_State
{
public:

    AwaitWifiState() : Window_State() {}
    ~AwaitWifiState() {}

    void enterState(State_Transfer_Data &transferData)
    {
        _WiFiConnectBegin = xTaskGetTickCount();
        Window_State::enterState(transferData);

        displayState();

        // Temporary
        if (ConnectivityModule::RadioUtils::ConnectToAccessPoint())
        {
            awaitingSmartConnect = false;
            skipTimeout = false;
            return;
        }

        bool credentialSaved = !(WiFi.SSID() == "");
        skipTimeout = true;
        awaitingSmartConnect = false;
        if (credentialSaved)
        {
            auto numNetworks = WiFi.scanNetworks();
            if (numNetworks > 0)
            {
                for (int i = 0; i < numNetworks; i++)
                {
                    if (WiFi.SSID() == WiFi.SSID(i))
                    {
                        skipTimeout = false;
                        WiFi.begin();
                        break;
                    }
                }
            }
        }
        
        if (skipTimeout)
        {
            // ConnectivityModule::RadioUtils::InitializeSmartConfig();
            // ConnectivityModule::Utilities::InitializeEspNow().Invoke(ConnectivityModule::Utilities::DataReceivedRpc, nullptr);
            ConnectivityModule::Utilities::InitializeWiFiProvisioning();
            awaitingSmartConnect = true;
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        if (!ConnectivityModule::RadioUtils::RadioState() != ConnectivityModule::RADIO_STATE_STA)
        {
            ConnectivityModule::Utilities::DeinitializeWiFiProvisioning();
            // WiFi.stopSmartConfig();
        }
    }

    void displayState()
    {
        if (!awaitingSmartConnect && (skipTimeout || xTaskGetTickCount() > _WiFiConnectBegin + _WiFiTimeOut))
        {
            ConnectivityModule::Utilities::InitializeWiFiProvisioning();
            awaitingSmartConnect = true;
        }

        if (!awaitingSmartConnect)
        {
            Display_Utils::printCenteredText("Connecting to WiFi...");
            Display_Utils::UpdateDisplay().Invoke();
        }
        else
        {
            auto provisioningMode = ConnectivityModule::Utilities::ProvisioningMode();

            if (provisioningMode == ConnectivityModule::WIFI_PROV_MODE_ESP_NOW)
            {
                TextFormat tf;
                tf.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
                tf.verticalAlignment = TEXT_LINE;
                tf.line = 2;
                Display_Utils::printFormattedText("No WiFi Found", tf);

                tf.line = 3;
                Display_Utils::printFormattedText("Awaiting SmartConfig", tf);
            }
            else if (provisioningMode == ConnectivityModule::WIFI_PROV_MODE_TEMP_AP)
            {
                TextFormat tf;
                tf.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
                tf.verticalAlignment = TEXT_LINE;
                tf.line = 1;
                Display_Utils::printFormattedText("Configure WiFi", tf);

                tf.line++;
                std::string ssid = "SSID: " + ConnectivityModule::Utilities::WiFiProvisiningApSSID();
                Display_Utils::printFormattedText(ssid.c_str(), tf);

                tf.line++;
                std::string password = "Password: " + ConnectivityModule::Utilities::WiFiProvisiningApPassword();
                Display_Utils::printFormattedText(password.c_str(), tf);
            }
            
            Display_Utils::UpdateDisplay().Invoke();

            ESP_LOGD(TAG, "SmartConfig status: %s", WiFi.smartConfigDone() ? "Done" : "Not Done");
        }
    }

private:
    TickType_t _WiFiConnectBegin = 0;
    TickType_t _WiFiTimeOut = 10000; 

    bool skipTimeout = true;
    bool awaitingSmartConnect;
};