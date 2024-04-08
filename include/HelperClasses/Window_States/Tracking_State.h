#pragma once

#include "Window_State.h"
#include "Tracking_Content.h"

class Tracking_State : public Window_State
{
public:
    Tracking_State(Tracking_Content *tracking) : trackingContent(tracking)
    {
        // typeID = __COUNTER__;
        renderContent = trackingContent;
        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
    }

    ~Tracking_State()
    {
        if (trackingContent != nullptr)
        {
            trackingContent->stop();
        }

        if (pingMsg != nullptr)
        {
            delete pingMsg;
            pingMsg = nullptr;
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Tracking_State::enterState");
#endif
        Window_State::enterState(transferData);

        if (trackingContent != nullptr)
        {
            trackingContent->start();

            if (transferData.serializedData != nullptr)
            {
                auto userID = (*transferData.serializedData)["userID"].as<uint64_t>();
                auto messageID = (*transferData.serializedData)["messageID"].as<uint32_t>();

                Message_Base *msg = Network_Manager::cloneMessageEntry(userID);
                if (msg != nullptr && msg->msgType == MESSAGE_PING)
                {
                    pingMsg = (Message_Ping *)(msg);
                }
                else
                {
                    if (msg != nullptr)
                    {
                        delete msg;
                    }
                    transferData.returnCode = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;

                    return;
                }

                trackingContent->assignMsg(pingMsg);
            }
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Tracking_State::exitState");
#endif
        Window_State::exitState(transferData);

        if (trackingContent != nullptr)
        {
            trackingContent->stop();
            trackingContent->unassignMsg();

            if (pingMsg != nullptr)
            {
                delete pingMsg;
                pingMsg = nullptr;
            }
        }

        transferData.serializedData = nullptr;
    }

private:
    Tracking_Content *trackingContent;
    Message_Ping *pingMsg = nullptr;
};