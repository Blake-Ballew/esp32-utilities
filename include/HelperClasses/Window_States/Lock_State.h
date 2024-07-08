#pragma once

#include "globalDefines.h"
#include "Window_State.h"
#include "LED_Manager.h"
#include "vector"

/// @brief The Lock_State class is a Window_State that clears the display and 
/// prompts the user to unlock the device with a sequence of button presses
class Lock_State : public Window_State
{
public:
    Lock_State()
    {
        inputSequence = {BUTTON_3, BUTTON_4};1
        currentInput = inputSequence.begin();
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
    }

    void unlock() 
    {
        DisplayCommandQueueItem queueItem;
        queueItem.commandType = CommandType::CALLBACK_COMMAND;
        queueItem.source = CommandSource::WINDOW;
        queueItem.commandData.callbackCommand.resourceID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;

        xQueueSend(OLED_Content::displayCommandQueue, &queueItem, 0);
    }
    
    std::vector<uint8_t>::iterator currentInput;
    std::vector<uint8_t> inputSequence;
};