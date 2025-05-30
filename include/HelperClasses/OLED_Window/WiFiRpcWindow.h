#pragma once

#include "OLED_Window.h"
#include "AwaitWifiState.h"
#include "IpUtils.h"
#include "RadioUtils.h"
#include "RpcUtils.h"

namespace
{

    const int RPC_AWAITING_CONNECTION = 0;
    const int RPC_CONNECTION_ESTABLISHED = 1;
    const int RPC_WIFI_FAILED = 2;
}

class WiFiRpcWindow : public OLED_Window
{
public:
    WiFiRpcWindow(OLED_Window *parent) : OLED_Window(parent) 
    {
        _wifiConnectState.assignInput(BUTTON_3, ACTION_BACK, "Back");
        _otherState.assignInput(BUTTON_3, ACTION_BACK, "Back");


        setInitialState(&_wifiConnectState);
        Display_Utils::enableRefreshTimer(500);
    }

    ~WiFiRpcWindow() 
    {
        if (ConnectivityModule::RadioUtils::IsRadioActive())
        {
            ConnectivityModule::RadioUtils::DisableRadio();
        }
    }

    void drawWindow() 
    {
        OLED_Window::drawWindow();

        if (currentState == &_wifiConnectState && ConnectivityModule::RadioUtils::IsWiFiActive())
        {
            State_Transfer_Data td;
            td.oldState = &_wifiConnectState;
            td.newState = &_otherState;

            transferState(td);
        }
        else if (currentState == &_otherState && !ConnectivityModule::RadioUtils::IsWiFiActive())
        {
            State_Transfer_Data td;
            td.oldState = &_otherState;
            td.newState = &_wifiConnectState;

            transferState(td);
        }
        else if (currentState == &_otherState)
        {
            std::string ip = std::string(WiFi.localIP().toString().c_str());
            std::string msg = "IP: " + ip;

            TextFormat prompt;
            prompt.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            prompt.verticalAlignment = ALIGN_CENTER_VERTICAL;

            Display_Utils::printFormattedText(msg.c_str(), prompt);
        }

        Display_Utils::UpdateDisplay().Invoke();
    }

protected:
    // State to connect to an AP via saved credentials or SmartConfig
    AwaitWifiState _wifiConnectState;
    
    // State that displays IP the user can connect to
    Window_State _otherState;

private:
    int _rpcConnectionState = RPC_AWAITING_CONNECTION;
};