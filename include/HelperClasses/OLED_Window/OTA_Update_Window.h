#pragma once

#include "OLED_Window.h"
#include "globalDefines.h"
#include "System_Utils.h"

class OTA_Update_Window : public OLED_Window
{
public:
    OTA_Update_Window(OLED_Window *parent) : OLED_Window(parent)
    {
        Window_State *state = new Window_State();
        currentState = state;
        state->assignInput(BUTTON_3, ACTION_BACK, "Back");

        stateList.push_back(currentState);
        Display_Utils::printCenteredText("Connecting to WiFi...");
        display->display();
        System_Utils::enableWiFi();
        System_Utils::startOTA();
    }

    ~OTA_Update_Window() {
        System_Utils::stopOTA();
        System_Utils::disableWiFi();
    }

    void drawWindow() override
    {
        OLED_Window::drawWindow();
        // Display_Utils::printCenteredText("Updating...");
        display->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(1));
        display->print("Waiting for firmware");
        display->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(2));
        display->print("IP: ");
        display->print(System_Utils::getLocalIP());
        display->display();
    }
};