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
    uint32_t returnCode = 0;
    Window_State *oldState = nullptr;
    Window_State *newState = nullptr;
};

class Window_State
{
public:
    inline static Adafruit_SSD1306 *display = nullptr;

    OLED_Content *renderContent = nullptr;

    // map inputID to callback struct
    // This will be assigned by this class in the constructor
    std::map<uint8_t, CallbackData> buttonCallbacks;

    // map inputID to adjacent state
    // This will be assigned by the Window class
    std::map<uint8_t, Window_State *> adjacentStates;

    Window_State()
    {
    }

    // Window should handle destroying content classes
    virtual ~Window_State() {}

    virtual void processInput(uint8_t inputID) {}

    /*
        Child classes overriding this method shouldn't care what inputID was used
        The callbackID can be used to know if the state was an adjacent transfer or a return from a function state
    */
    virtual void enterState(State_Transfer_Data &transferData)
    {

        if (renderContent != nullptr)
            renderContent->start();
    }

    // Used when window is closing
    virtual void exitState()
    {

        if (renderContent != nullptr)
            renderContent->stop();
    }

    /*
        Child classes overriding this method will have outputs configured for certain inputIDs
        The state being entered will receive these outputs as input
        A state transferring to a state expecting a certain input should supply that output
        If the callbackID is ACTION_RETURN_FROM_FUNCTIONAL_STATE, the state will output the return value of the function state
    */
    virtual void exitState(State_Transfer_Data &transferData)
    {

        if (renderContent != nullptr)
            renderContent->stop();
    }

    virtual void displayState()
    {
#if DEBUG == 1
        Serial.println("Window_State::displayState");
#endif
        if (renderContent != nullptr)
            renderContent->printContent();
    }

    void assignInput(uint8_t inputID, uint32_t callbackID, const char *displayText)
    {
        CallbackData callback;
        callback.callbackID = callbackID;
        strncpy(callback.displayText, displayText, BUTTON_TEXT_MAX);
        callback.displayText[min(strlen(displayText), (size_t)BUTTON_TEXT_MAX)] = '\0';
        buttonCallbacks[inputID] = callback;
    }

    void assignInput(uint8_t inputID, CallbackData &callback)
    {
        buttonCallbacks[inputID] = callback;
    }

    void assignInput(uint8_t inputID, uint32_t callbackID)
    {
        CallbackData callback;
        callback.callbackID = callbackID;
        buttonCallbacks[inputID] = callback;
    }

    void setAdjacentState(uint8_t inputID, Window_State *state)
    {
        adjacentStates[inputID] = state;
    }

    void clearAdjacentState(uint8_t inputID)
    {
        if (adjacentStates.find(inputID) != adjacentStates.end())
            adjacentStates.erase(inputID);
    }

    Window_State *getAdjacentState(uint8_t inputID)
    {
        if (adjacentStates.find(inputID) != adjacentStates.end())
            return adjacentStates[inputID];
        return nullptr;
    }

protected:
};
