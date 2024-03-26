#pragma once

#include "globalDefines.h"
#include "OLED_Content.h"
#include <ArduinoJson.h>
#include <map>

class Function_State;
class Window_State;

struct State_Transfer_Data
{
    uint8_t inputID = 0;
    size_t callbackID = ACTION_NONE;
    ArduinoJson::DynamicJsonDocument *serializedData = nullptr;
};

class Window_State
{
public:
    static Adafruit_SSD1306 *display;

    OLED_Content *renderContent;

    // map inputID to callback struct
    // This will be assigned by this class in the constructor
    std::map<uint8_t, CallbackData> buttonCallbacks;

    // map inputID to adjacent state
    // This will be assigned by the Window class
    std::map<uint8_t, Window_State *> adjacentStates;

    Window_State() {}

    // Window should handle destroying content classes
    virtual ~Window_State() {}

    virtual void processInput(uint8_t inputID) {}

    virtual void enterState(State_Transfer_Data &transferData)
    {
        if (renderContent != nullptr)
            renderContent->start();
    }

    virtual void exitState()
    {
        if (renderContent != nullptr)
            renderContent->stop();
    }

    virtual void exitState(State_Transfer_Data &transferData)
    {
        if (renderContent != nullptr)
            renderContent->stop();

        transferData.serializedData = nullptr;
    }

    virtual void displayState()
    {
        if (renderContent != nullptr)
            renderContent->printContent();
    }
};
