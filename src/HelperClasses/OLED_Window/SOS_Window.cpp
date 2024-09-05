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
        MessagePing *ping = createOkayMessage();

        // Send message
        LoraUtils::SendMessage(ping, 1);

        delete ping;
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

    if (transferData.newState == sosState)
    {
        // Send SOS message
        MessagePing *ping = createSosMessage();

        transferData.serializedData = new ArduinoJson::DynamicJsonDocument(512);
        ping->serialize(*transferData.serializedData);
        transferData.serializedData->shrinkToFit();

        delete ping;
    }

    transferData.newState->enterState(transferData);

    // processStateChangeReturnCode(transferData.returnCode);

    currentState = transferData.newState;

    // Clean up
    if (transferData.serializedData != nullptr)
    {
        delete transferData.serializedData;
    }
}

MessagePing *SOS_Window::createSosMessage()
{
    NavigationUtils::UpdateGPS();

    MessagePing *ping = new MessagePing(
        NavigationUtils::GetTime().value(),
        NavigationUtils::GetDate().value(),
        0,   // Recipient
        LoraUtils::UserID(),
        LoraUtils::UserName().c_str(),
        0,   // MsgID
        255, // Red
        0,   // Green
        0,   // Blue
        NavigationUtils::GetLocation().lat(),
        NavigationUtils::GetLocation().lng(),
        "SOS"
    );
    
    return ping;
}

MessagePing *SOS_Window::createOkayMessage()
{
    NavigationUtils::UpdateGPS();

    MessagePing *ping = new MessagePing(
        NavigationUtils::GetTime().value(),
        NavigationUtils::GetDate().value(),
        0,
        LoraUtils::UserID(),
        LoraUtils::UserName().c_str(),
        0,
        0,
        255,
        0,
        NavigationUtils::GetLocation().lat(),
        NavigationUtils::GetLocation().lng(),
        "OK"
    );
    
    return ping;
}


