#pragma once

#include "globalDefines.h"
#include "Window_State.h"
#include "LED_Manager.h"
#include "vector"
#include "Illuminate_Button.h"
#include "LED_Utils.h"

/// @brief The Lock_State class is a Window_State that clears the display and 
/// prompts the user to unlock the device with a sequence of button presses
class Lock_State : public Window_State
{
public:
    Lock_State()
    {
        inputSequence = {BUTTON_3, BUTTON_4};
        currentInput = inputSequence.begin();

        if (Illuminate_Button::registeredPatternID == -1) {
            Illuminate_Button *illuminate = new Illuminate_Button(LED_Utils::getInputLedPairs());
            illuminateID = LED_Utils::registerPattern(illuminate);
        }
        else 
        {
            illuminateID = Illuminate_Button::registeredPatternID;
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);
        resetLock();
        
    }

    void initializeInputSequence(std::vector<uint8_t> sequence)
    {
        inputSequence = sequence;
        resetLock();
    }

    void processInput(uint8_t inputID)
    {
        if (inputID == *currentInput)
        {
            currentInput++;
            if (currentInput == inputSequence.end())
            {
                unlock();
            }
        }
        else
        {
            resetLock();
        }
    }

protected:
    void resetLock() 
    {
        currentInput = inputSequence.begin();

        StaticJsonDocument<200> cfg;
        auto array = cfg.createNestedArray("inputStates");

        cfg["inputStates"][0]["input"] = *currentInput;
        cfg["inputStates"][0]["state"] = true;

        LED_Utils::configurePattern(illuminateID, cfg.as<JsonObject>());
        LED_Utils::iteratePattern(illuminateID);
    }

    void unlock() 
    {
        Display_Utils::sendCallbackCommand(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE);
    }
    
    std::vector<uint8_t>::iterator currentInput;
    std::vector<uint8_t> inputSequence;
    int illuminateID;
};