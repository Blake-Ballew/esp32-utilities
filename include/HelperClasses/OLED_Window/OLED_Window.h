#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <stack>
#include <vector>
#include "OLED_Content.h"
#include "LED_Manager.h"
#include "Window_State.h"
#include "globalDefines.h"
// #include "OLED_Manager.h"

#define BUTTON_TEXT_MAX 12

class OLED_Window
{
public:
    OLED_Window();
    OLED_Window(OLED_Window *parent);

    static Adafruit_SSD1306 *display;
    OLED_Content *content = nullptr;
    Window_State *currentState = nullptr;
    std::stack<Window_State *> stateStack;

    virtual void drawWindow();
    void assignButton(uint32_t callbackID, int buttonNumber, const char *buttonText, uint8_t textLength);

    OLED_Window *getParentWindow();

    // Only use in child class
    virtual void execBtnCallback(uint8_t inputID);
    virtual void Pause()
    {
        if (currentState != nullptr && currentState->renderContent != nullptr)
        {
            currentState->renderContent->stop();
        }
    }
    virtual void Resume()
    {
        if (currentState != nullptr && currentState->renderContent != nullptr)
        {
            currentState->renderContent->start();
        }
    }

    virtual void encUp()
    {
        if (content != NULL)
            content->encUp();
    }
    virtual void encDown()
    {
        if (content != NULL)
            content->encDown();
    }

    virtual CallbackData *getCallbackDataByInputID(uint8_t inputID)
    {
        if (this->currentState == nullptr)
        {
            return nullptr;
        }

        if (this->currentState->buttonCallbacks.find(inputID) == this->currentState->buttonCallbacks.end())
        {
            return nullptr;
        }

        return &this->currentState->buttonCallbacks[inputID];
    }

    virtual void switchContent(OLED_Content *content, bool copyButtons)
    {
        if (content == nullptr)
        {
            return;
        }

        this->content = content;
    }

    // Switches state to the adjacent state with the given inputID
    virtual void switchWindowState(uint8_t inputID)
    {
        if (currentState == nullptr)
        {
            return;
        }

        auto itNextState = currentState->adjacentStates.find(inputID);

        if (itNextState == currentState->adjacentStates.end())
        {
            return;
        }

        // Get next state
        Window_State *prevState = currentState;
        Window_State *newState = currentState->adjacentStates[inputID];

        // Setup transfer data
        State_Transfer_Data transferData;
        transferData.inputID = inputID;
        transferData.callbackID = ACTION_SWITCH_WINDOW_STATE;
        transferData.serializedData = nullptr;
        transferData.oldState = prevState;
        transferData.newState = newState;

        // Transfer
        transferState(transferData);
    }

    // Switches state to the adjacent state with the given inputID
    // The current state is pushed onto the stack to be returned to later
    virtual void callFunctionState(uint8_t inputID)
    {
        if (currentState == nullptr)
        {
            return;
        }

        // get next state

        auto itNextState = currentState->adjacentStates.find(inputID);

        if (itNextState == currentState->adjacentStates.end())
        {
            return;
        }

        // Save current state
        stateStack.push(currentState);

        // Get next state
        Window_State *prevState = currentState;
        Window_State *newState = currentState->adjacentStates[inputID];

        // Setup transfer data
        State_Transfer_Data transferData;
        transferData.inputID = inputID;
        transferData.callbackID = ACTION_CALL_FUNCTIONAL_WINDOW_STATE;
        transferData.serializedData = nullptr;
        transferData.oldState = prevState;
        transferData.newState = newState;

        transferState(transferData);
    }

    virtual void returnFromFunctionState(uint8_t inputID)
    {
#if DEBUG == 1
        Serial.println("Returning from function state");
#endif
        if (stateStack.empty())
        {
#if DEBUG == 1
            Serial.println("State stack is empty");
#endif
            return;
        }

        // Get next state
        Window_State *prevState = currentState;
        Window_State *newState = stateStack.top();
        stateStack.pop();

        // Setup transfer data
        State_Transfer_Data transferData;
        transferData.inputID = inputID;
        transferData.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        transferData.serializedData = nullptr;
        transferData.oldState = prevState;
        transferData.newState = newState;

        // Transfer
        transferState(transferData);
    }

    void setInitialState(Window_State *initialState)
    {
        currentState = initialState;
        State_Transfer_Data transferData;
        transferData.inputID = 0;
        transferData.callbackID = ACTION_NONE;
        transferData.serializedData = nullptr;

        currentState->enterState(transferData);
    }

    // void execBtnCallback(uint8_t buttonNumber, void *arg);

    virtual ~OLED_Window();

    bool isPaused = false;
    bool allowInterrupts = false;

    uint32_t btn1CallbackID = 0;
    uint32_t btn2CallbackID = 0;
    uint32_t btn3CallbackID = 0;
    uint32_t btn4CallbackID = 0;

    // ArduinoJson::JsonDocument callbackData;

protected:
    // Exits the old state and sends its transfer data to the new state
    // After entering the new state, the return code is read
    virtual void transferState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("OLED_Window::transferState()");
#endif

        transferData.oldState->exitState(transferData);

        // Child classes will process transfer data coming out of the old state if needed

        transferData.newState->enterState(transferData);

        // processStateChangeReturnCode(transferData.returnCode);

        currentState = transferData.newState;

        // Clean up
        if (transferData.serializedData != nullptr)
        {
            delete transferData.serializedData;
        }
    }

    virtual void processStateChangeReturnCode(size_t returnCode)
    {
        switch (returnCode)
        {
        case ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE:
            returnFromFunctionState(0);
            break;
        default:
            break;
        }
    }

    void (*btnCallback)(uint8_t, OLED_Window *);

    // These will be used to auto delete the content and state classes
    std::vector<Window_State *> stateList;
    std::vector<OLED_Content *> contentList;

    size_t returnAction = 0;

    char btn1Text[BUTTON_TEXT_MAX + 1];
    char btn2Text[BUTTON_TEXT_MAX + 1];
    char btn3Text[BUTTON_TEXT_MAX + 1];
    char btn4Text[BUTTON_TEXT_MAX + 1];

    OLED_Window *parentWindow;
};
