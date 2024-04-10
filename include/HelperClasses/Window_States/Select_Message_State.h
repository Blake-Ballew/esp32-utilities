#pragma once

#include "Window_State.h"
#include "Saved_Messages_Content.h"
#include "Network_Manager.h"
#include "Navigation_Manager.h"

// namespace
// {
//     const char *MESSAGE_SENT PROGMEM = "Message sent";
//     const char *NO_ROUTE PROGMEM = "No route";
//     const char *DELIVERY_FAILED PROGMEM = "Delivery failed";
//     const char *UNABLE_TO_QUEUE PROGMEM = "Unable to queue";
// }

/*
    Takes in a set of coordinates to be sent as a broadcast or direct message.
    The user can select a message to send from a list of saved messages or
    use the name of the location as the message.
*/

class Select_Message_State : public Window_State
{
public:
    // edit bool decides if state is used to manage saved messages or send them.
    Select_Message_State(Saved_Messages_Content *content)
    {
        msgContent = content;
        renderContent = msgContent;

        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Send");

        msgContent->setSelectMsgPrompt();
    }

    Select_Message_State()
    {
        Select_Message_State(new Saved_Messages_Content());
    }

    ~Select_Message_State()
    {
        if (msgContent != nullptr)
        {
            msgContent->stop();
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Select_Message_State::enterState");
#endif
        // Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr && transferData.serializedData->containsKey("additionalMsgList"))
        {
            for (auto msg : (*transferData.serializedData)["additionalMsgList"].as<JsonArray>())
            {
#if DEBUG == 1
                Serial.print("Adding message: ");
                Serial.println(msg.as<const char *>());
#endif
                msgContent->insertMessage(msg.as<const char *>());
            }
        }

        msgContent->loadMessages();
    }

    void exitState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Select_Message_State::exitState");
#endif
        Window_State::exitState(transferData);

        if (transferData.inputID == BUTTON_4)
        {
            auto message = msgContent->getCurrentMessage();
            if (message != nullptr)
            {
                DynamicJsonDocument *doc = new DynamicJsonDocument(128);
                (*doc)["message"] = message;
                transferData.serializedData = doc;
            }
        }

        msgContent->clearAdditionalMessages();

        LED_Manager::clearRing();
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
        {
            msgContent->encUp();
        }
        break;
        case ENC_DOWN:
        {
            msgContent->encDown();
        }
        break;
        default:
            break;
        }
    }

private:
    Saved_Messages_Content *msgContent;

    bool sendDirect;
    uint64_t userID;
};
