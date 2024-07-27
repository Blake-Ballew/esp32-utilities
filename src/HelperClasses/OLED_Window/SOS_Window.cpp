#include "SOS_Window.h"

SOS_Window::SOS_Window(OLED_Window *parent, 
Repeat_Message_State *repeat,
Lock_State *lock) : OLED_Window(parent)
{
    sosState = repeat;
    lockState = lock;
    
    stateList.push_back(sosState);
    stateList.push_back(lockState);

    if (sosState->renderContent != nullptr)
        contentList.push_back(sosState->renderContent);
    
    if (lockState->renderContent != nullptr)
        contentList.push_back(lockState->renderContent);

    std::vector<uint8_t> inputList = {BUTTON_1, BUTTON_2, BUTTON_4};
    lockState->assignInput(BUTTON_1, ACTION_DEFER_CALLBACK_TO_WINDOW);
    lockState->assignInput(BUTTON_2, ACTION_DEFER_CALLBACK_TO_WINDOW);
    lockState->assignInput(BUTTON_4, ACTION_DEFER_CALLBACK_TO_WINDOW);
    lockState->initializeInputSequence(inputList);

    setInitialState(lockState);

    // Force lock state to return to SOS state
    stateStack.push(sosState);
}

SOS_Window::~SOS_Window()
{

}

void SOS_Window::execBtnCallback(uint8_t inputID)
{
    OLED_Window::execBtnCallback(inputID);

    if (currentState == sosState && currentState->buttonCallbacks[inputID].callbackID == ACTION_BACK) 
    {
        // Send okay message
    }
}

void SOS_Window::Pause()
{
    
}

void SOS_Window::Resume()
{
    
}

void SOS_Window::transferState(State_Transfer_Data &transferData)
{
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


