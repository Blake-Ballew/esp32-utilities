#pragma once

#include "Window_State.h"
#include "RadioUtils.h"

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
        ConnectivityModule::RadioUtils::ConnectToAccessPoint();
        awaitingSmartConnect = false;
        skipTimeout = false;
        return;

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
            ConnectivityModule::RadioUtils::InitializeSmartConfig();
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        if (!WiFi.smartConfigDone())
        {
            WiFi.stopSmartConfig();
        }
    }

    void displayState()
    {
        if (!awaitingSmartConnect && (skipTimeout || xTaskGetTickCount() > _WiFiConnectBegin + _WiFiTimeOut))
        {
            ConnectivityModule::RadioUtils::InitializeSmartConfig();
            awaitingSmartConnect = true;
        }

        if (!awaitingSmartConnect)
        {
            Display_Utils::printCenteredText("Connecting to WiFi...");
            Display_Utils::UpdateDisplay().Invoke();
        }
        else
        {
            TextFormat tf;
            tf.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            tf.verticalAlignment = TEXT_LINE;
            tf.line = 2;
            Display_Utils::printFormattedText("No WiFi Found", tf);

            tf.line = 3;
            Display_Utils::printFormattedText("Awaiting SmartConfig", tf);
            Display_Utils::UpdateDisplay().Invoke();

            #if DEBUG == 1
            WiFi.smartConfigDone() ? Serial.println("SmartConfig Done") : Serial.println("SmartConfig Not Done");
            #endif
        }
    }

private:
    TickType_t _WiFiConnectBegin = 0;
    TickType_t _WiFiTimeOut = 10000; 

    bool skipTimeout = true;
    bool awaitingSmartConnect;
};