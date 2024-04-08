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

        assignInput(BUTTON_3, ACTION_BACK, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Send");
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
    }

    void processInput(uint8_t inputID)
    {
        /* if (inputID == BUTTON_4)
        {
            if (sendCurrentLocation)
            {
                Navigation_Manager::updateGPS();
                auto location = Navigation_Manager::getLocation();
                lat = location.lat();
                lon = location.lng();
            }

            Message_Ping *message = new Message_Ping(
                Navigation_Manager::getTime().value(),
                Navigation_Manager::getDate().value(),
                userID,
                Network_Manager::userID,
                Settings_Manager::settings["User"]["Name"]["cfgVal"].as<const char *>(),
                0,
                Settings_Manager::settings["User"]["Theme Red"]["cfgVal"].as<uint8_t>(),
                Settings_Manager::settings["User"]["Theme Green"]["cfgVal"].as<uint8_t>(),
                Settings_Manager::settings["User"]["Theme Blue"]["cfgVal"].as<uint8_t>(),
                lat,
                lon,
                msgContent->getCurrentMessage());

            uint8_t returnCode;

            if (sendDirect)
            {
                returnCode = Network_Manager::queueMessageToUser(userID, message);
            }
            else
            {
                returnCode = Network_Manager::queueBroadcastMessage(message);
            }
            OLED_Content::clearContentArea();

            switch (returnCode)
            {
            case RETURN_CODE_UNABLE_TO_QUEUE:
                display->setCursor(OLED_Content::centerTextHorizontal(UNABLE_TO_QUEUE), OLED_Content::centerTextVertical());
                display->print(UNABLE_TO_QUEUE);
                break;
            case RH_ROUTER_ERROR_NONE:
                display->setCursor(OLED_Content::centerTextHorizontal(MESSAGE_SENT), OLED_Content::centerTextVertical());
                display->print(MESSAGE_SENT);
                break;
            case RH_ROUTER_ERROR_NO_ROUTE:
                display->setCursor(OLED_Content::centerTextHorizontal(NO_ROUTE), OLED_Content::centerTextVertical());
                display->print(NO_ROUTE);
                break;
            case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
                display->setCursor(OLED_Content::centerTextHorizontal(DELIVERY_FAILED), OLED_Content::centerTextVertical());
                display->print(DELIVERY_FAILED);
                break;
            default:
                break;
            }

            display->display();

            vTaskDelay(pdMS_TO_TICKS(3000));
    }*/
    }

private:
    Saved_Messages_Content *msgContent;

    bool sendDirect;
    uint64_t userID;
};
