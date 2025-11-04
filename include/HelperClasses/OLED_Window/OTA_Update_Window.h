#pragma once

#include "OLED_Window.h"
#include "AwaitWifiState.h"
#include "globalDefines.h"
#include "System_Utils.h"

class OTA_Update_Window : public OLED_Window
{
public:
    OTA_Update_Window(OLED_Window *parent) : OLED_Window(parent)
    {
        _wifiConnectState.assignInput(BUTTON_3, ACTION_BACK, "Back");
        _otherState.assignInput(BUTTON_3, ACTION_BACK, "Back");
        setInitialState(&_wifiConnectState);
        System_Utils::startOTA();
    }
    
    ~OTA_Update_Window() {
        System_Utils::stopOTA();
        if (ConnectivityModule::RadioUtils::IsRadioActive())
        {
            ConnectivityModule::RadioUtils::DisableRadio();
        }
    }

    void drawWindow() override
    {
        OLED_Window::drawWindow();
        // Display_Utils::printCenteredText("Updating...");
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
            display->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(1));
            display->print("Waiting for firmware");
            display->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(2));
            display->print("IP: ");
            display->print(System_Utils::getLocalIP());
            display->display();
        }
    }

private:
    // State to connect to an AP via saved credentials or SmartConfig
    AwaitWifiState _wifiConnectState;

    // State that displays IP the user can connect to
    Window_State _otherState;
};