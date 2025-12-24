#pragma once

#include "globalDefines.h"
#include "Window_State.h"
#include "LED_Manager.h"
#include "vector"
#include "Illuminate_Button.h"
#include "Button_Flash.h"
#include "LED_Utils.h"


/// @brief The Lock_State class is a Window_State that clears the display and
/// prompts the user to unlock the device with a sequence of button presses.
/// It can also be used any time a sequence of button presses is required.
class Lock_State : public Window_State
{
public:
    Lock_State()
    {
        allowInterrupts = false;

        inputSequence = {BUTTON_3, BUTTON_4};
        currentInput = inputSequence.begin();

        
    }

    void enterState(State_Transfer_Data &transferData)
    {
        illuminateID = Illuminate_Button::RegisteredPatternID();
        
        Window_State::enterState(transferData);
        
        LED_Utils::disablePattern(Button_Flash::RegisteredPatternID());
        LED_Utils::enablePattern(illuminateID);

        resetLock();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);
        LED_Utils::disablePattern(illuminateID);
        LED_Utils::enablePattern(Button_Flash::RegisteredPatternID());
    }

    void initializeInputSequence(std::vector<uint8_t> sequence)
    {
        inputSequence = sequence;
        resetLock();
    }

    void processInput(uint8_t inputID)
    {
        if (buttonCallbacks.find(inputID) != buttonCallbacks.end() &&
            buttonCallbacks[inputID].callbackID == ACTION_BACK)
            {
                resetLock();
                LED_Utils::clearPattern(illuminateID);
                return;
            }

        if (inputID == *currentInput)
        {
            ESP_LOGD(TAG, "Lock_State::processInput inputID: %d", inputID);
            StaticJsonDocument<200> cfg;
            auto array = cfg.createNestedArray("inputStates");

            cfg["inputStates"][0]["input"] = *currentInput;
            cfg["inputStates"][0]["state"] = false;

            currentInput++;
            
            if (currentInput == inputSequence.end())
            {
                LED_Utils::configurePattern(illuminateID, cfg); 
                LED_Utils::iteratePattern(illuminateID);

                unlock();
            }
            else
            {
                cfg["inputStates"][1]["input"] = *currentInput;
                cfg["inputStates"][1]["state"] = true;

                LED_Utils::configurePattern(illuminateID, cfg);
                LED_Utils::iteratePattern(illuminateID);
            }
        }
        else
        {
            ESP_LOGD(TAG, "Lock_State::processInput resetting inputID: %d", inputID);
            resetLock();
        }
    }

    

protected:
    void resetLock() 
    {
        
        StaticJsonDocument<256> cfg;
        auto array = cfg.createNestedArray("inputStates");

        cfg["inputStates"][0]["input"] = *currentInput;
        cfg["inputStates"][0]["state"] = false;

        currentInput = inputSequence.begin();

        cfg["inputStates"][1]["input"] = *currentInput;
        cfg["inputStates"][1]["state"] = true;

        LED_Utils::configurePattern(illuminateID, cfg);
        LED_Utils::iteratePattern(illuminateID);
    }

    void unlock() 
    {
        Display_Utils::sendCallbackCommand(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Window_State *nextState;
    
    std::vector<uint8_t>::iterator currentInput;
    std::vector<uint8_t> inputSequence;
    int illuminateID;
};