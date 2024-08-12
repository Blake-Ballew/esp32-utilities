#pragma once

#include "Window_State.h"
#include "Received_Messages_Content.h"

class Received_Messages_State : public Window_State
{
public:
    Received_Messages_State(Received_Messages_Content *statuses) : statusesContent(statuses)
    {
        renderContent = statusesContent;

        if (statusesContent != nullptr)
        {
            // TODO: Move this control to the window
            wrapAround = statusesContent->getWrapAround();

            assignInput(ENC_DOWN, ACTION_DEFER_CALLBACK_TO_WINDOW);

            if (!wrapAround)
            {
                // This is used for the home window
                assignInput(ENC_UP, ACTION_SWITCH_WINDOW_STATE);
                assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Reply");
                assignInput(BUTTON_1, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Track");
                assignInput(BUTTON_3, ACTION_DEFER_CALLBACK_TO_WINDOW, "Mark Read");
            }
            else
            {
                // this is used for received messages window
                assignInput(BUTTON_3, ACTION_BACK, "Back");
                assignInput(ENC_UP, ACTION_DEFER_CALLBACK_TO_WINDOW);
            }
            
        }
    }

    ~Received_Messages_State()
    {
    }

    void enterState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Received_Messages_State::enterState");
#endif
        Window_State::enterState(transferData);

        if (statusesContent->ShowReadMsgs())
        {
            LoraUtils::ResetMessageIterator();
        }
        else
        {
            LoraUtils::ResetUnreadMessageIterator();
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Received_Messages_State::exitState");
#endif
        Window_State::exitState(transferData);

        // Button 1: Tracking State: (userID, messageID)
        // Button 2: Ping State (false): Only when showRead is true
        // Button 3: Ping State (true, userID)
        // ENC_UP: Home State
        switch (transferData.inputID)
        {
        case BUTTON_1:
        {
            auto userID = statusesContent->getCurrentMessage()->sender;
            auto messageID = statusesContent->getCurrentMessage()->msgID;
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["userID"] = userID;
            (*doc)["messageID"] = messageID;
            transferData.serializedData = doc;
        }
        break;
        case BUTTON_2:
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["isDirect"] = false;
            transferData.serializedData = doc;
            break;
        }
        case BUTTON_3:
        {
            auto userID = statusesContent->getCurrentMessage()->sender;
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["isDirect"] = true;
            (*doc)["recipientID"] = userID;
            transferData.serializedData = doc;
            break;
        }
        case ENC_UP:
        {
            transferData.serializedData = nullptr;
            break;
        }
        default:
            break;
        }
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case BUTTON_1:
            break;
        case BUTTON_2:
            if (buttonCallbacks.find(BUTTON_2) != buttonCallbacks.end() &&
                buttonCallbacks[BUTTON_2].callbackID == ACTION_DEFER_CALLBACK_TO_WINDOW)
            {
                statusesContent->markMessageAsRead();
            }
            break;
        case BUTTON_3:
            break;
        case BUTTON_4:
            break;
        case ENC_DOWN:
        {
            statusesContent->encDown();
            break;
        }
        case ENC_UP:
        {
            statusesContent->encUp();
            break;
        }
        default:
            break;
        }
    }

    void enableButton2MarkRead()
    {
        assignInput(BUTTON_2, ACTION_DEFER_CALLBACK_TO_WINDOW, "Mark Read");
    }

private:
    Received_Messages_Content *statusesContent;
    bool wrapAround = false;
};