#pragma once

#include "globalDefines.h"
#include "OLED_Window.h"
#include "Menu_State.h"

class Menu_Window : public OLED_Window
{
public:
    Menu_Window(OLED_Window *parent) : OLED_Window(parent)
    {
        menuState = new Menu_State();
        currentState = menuState;

        

        stateList.push_back(currentState);
    }

    ~Menu_Window() {}
    
    void callFunctionState(uint8_t inputID)
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
        Window_State *newState = menuState->getAdjacentState();

        // Setup transfer data
        State_Transfer_Data transferData;
        transferData.inputID = inputID;
        transferData.callbackID = ACTION_CALL_FUNCTIONAL_WINDOW_STATE;
        transferData.serializedData = nullptr;
        transferData.oldState = prevState;
        transferData.newState = newState;

        transferState(transferData);
    }

    CallbackData *getCallbackDataByInputID(uint8_t inputID) 
    {
        #if DEBUG == 1
        Serial.println("Menu_Window::getCallbackDataByInputID");
        #endif
        CallbackData *callbackData = OLED_Window::getCallbackDataByInputID(inputID);

        #if DEBUG == 1
        Serial.printf("Menu_Window::getCallbackDataByInputID: inputID: %d\n", inputID);
        #endif

        if (callbackData != nullptr && callbackData->callbackID == ACTION_SELECT) {
            auto menuCallback = menuState->getMenuItemCallback();
            return menuCallback;
        }
        else
        {
            return callbackData;
        }
    }

    void callFunctionState(uint8_t inputID)
    {
        if (menuState->getAdjacentState() != nullptr)
        {
            stateStack.push(currentState);

            State_Transfer_Data transferData;
            transferData.inputID = inputID;
            transferData.callbackID = ACTION_CALL_FUNCTIONAL_WINDOW_STATE;
            transferData.serializedData = nullptr;
            transferData.oldState = currentState;
            transferData.newState = menuState->getAdjacentState();

            transferState(transferData);
        }
    }

    void addMenuItem(const char *text, uint32_t callbackID, Window_State *adjacentState, bool addAdjacentState = true)
    {
        menuState->addMenuItem(text, callbackID, adjacentState);

        if (addAdjacentState) stateList.push_back(adjacentState);
    }

    void addMenuItem(const char *text, uint32_t callbackID)
    {
        menuState->addMenuItem(text, callbackID);
    }

    Menu_State *menuState;

protected:


};