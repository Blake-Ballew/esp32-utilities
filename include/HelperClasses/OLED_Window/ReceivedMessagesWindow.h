#pragma once

#include "OLED_Window.h"
#include "ReceivedMessagesState.h"
#include "LoraMessageDisplay.h"
#include "Select_Message_State.h"
#include "Select_Location_State.h"
#include "SelectKeyValueState.h"
#include "SaveLocationState.h"
#include "Tracking_State.h"

namespace
{
    const char *CURR_LOCATION PROGMEM = "Current Location";
}

class ReceivedMessagesWindow : public OLED_Window
{
public:
    ReceivedMessagesWindow(OLED_Window *parent) : OLED_Window(parent) 
    {
        LoraMessageDisplay *messageDisplay = new LoraMessageDisplay();
        
        receivedMessagesState = new ReceivedMessagesState(messageDisplay);
        trackingState = new Tracking_State(messageDisplay);
        selectMessageState = new Select_Message_State();
        selectLocationState = new Select_Location_State();
        selectionState = new SelectKeyValueState();
        saveLocationState = new SaveLocationState();
        
        contentList.push_back(messageDisplay);
        
        stateList.push_back(receivedMessagesState);
        stateList.push_back(trackingState);
        stateList.push_back(selectMessageState);
        stateList.push_back(selectLocationState);
        stateList.push_back(selectionState);
        stateList.push_back(saveLocationState);

        setInitialState(receivedMessagesState);

        receivedMessagesState->assignInput(ENC_UP, ACTION_DEFER_CALLBACK_TO_WINDOW);
        receivedMessagesState->assignInput(ENC_DOWN, ACTION_DEFER_CALLBACK_TO_WINDOW);
        receivedMessagesState->assignInput(BUTTON_1, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Save");
        receivedMessagesState->assignInput(BUTTON_2, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Reply");
        receivedMessagesState->assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Track");

        receivedMessagesState->setAdjacentState(BUTTON_1, selectionState);
        receivedMessagesState->setAdjacentState(BUTTON_2, selectLocationState);
        receivedMessagesState->setAdjacentState(BUTTON_4, trackingState);

        selectLocationState->setAdjacentState(BUTTON_4, selectMessageState);
        selectLocationState->assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Select");

        trackingState->assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
    }

    ~ReceivedMessagesWindow() {}

    void transferState(State_Transfer_Data &transferData)
    {
        Window_State *oldState = transferData.oldState;
        Window_State *newState = transferData.newState;

        oldState->exitState(transferData);

        if (oldState == receivedMessagesState && newState == selectLocationState)
        {
            auto *doc = transferData.serializedData;
            recipientID = (*doc)["recipientID"].as<uint32_t>();
        }
        else if (oldState == selectLocationState && transferData.serializedData != nullptr)
        {
            DynamicJsonDocument *doc = (DynamicJsonDocument *)transferData.serializedData;

            if (doc->containsKey("name")
            && doc->containsKey("lat")
            && doc->containsKey("lng"))
            {
                locName = (*doc)["name"].as<std::string>();
                latitude = (*doc)["lat"].as<double>();
                longitude = (*doc)["lng"].as<double>();
                
                std::string currLocCompare = std::string(CURR_LOCATION);
                if (locName == currLocCompare)
                {
                    useCurrLocation = true;
                }
                else
                {
                    useCurrLocation = false;
                }
            }

             // Transfer to Select Message State
            delete transferData.serializedData;
            transferData.serializedData = new DynamicJsonDocument(1024);

            JsonArray msgArray = (*transferData.serializedData).createNestedArray("messages");

            if (!useCurrLocation)
            {
                msgArray.add(locName);
            }

            LoraUtils::SerializeSavedMessageList(*transferData.serializedData);

            newState = selectMessageState;
        }
        else if (oldState == selectMessageState && transferData.serializedData != nullptr)
        {
            if (useCurrLocation)
            {
                NavigationUtils::UpdateGPS();
                auto loc = NavigationUtils::GetLocation();

                latitude = loc.lat();
                longitude = loc.lng();
            }

            DynamicJsonDocument *doc = (DynamicJsonDocument *)transferData.serializedData;
            if (doc != nullptr && doc->containsKey("message"))
            {
                const char *message = (*doc)["message"].as<const char *>();
                // Send message

                MessagePing *newMsg = new MessagePing(
                    NavigationUtils::GetTime().value(),
                    NavigationUtils::GetDate().value(),
                    recipientID,
                    LoraUtils::UserID(),
                    Settings_Manager::settings["User"]["Name"]["cfgVal"].as<const char *>(),
                    0,
                    Settings_Manager::settings["User"]["Theme Red"]["cfgVal"].as<uint8_t>(),
                    Settings_Manager::settings["User"]["Theme Green"]["cfgVal"].as<uint8_t>(),
                    Settings_Manager::settings["User"]["Theme Blue"]["cfgVal"].as<uint8_t>(),
                    latitude,
                    longitude,
                    message);

                uint8_t returnCode;

                auto success = LoraUtils::SendMessage(newMsg, 0);

                display->clearDisplay();
                
                if (success)
                {
                    char displayMsg[] = "Message sent";
                    display->setCursor(Display_Utils::centerTextHorizontal(displayMsg), Display_Utils::centerTextVertical());
                    display->print(displayMsg);
                }
                else
                {
                    char displayMsg[] = "Unable to send";
                    display->setCursor(Display_Utils::centerTextHorizontal(displayMsg), Display_Utils::centerTextVertical());
                    display->print(displayMsg);
                }

                display->display();
                clearMessageInfo();
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                newState = receivedMessagesState;
            }
        }
        else if (oldState == receivedMessagesState && newState == selectionState)
        {
            transferData.serializedData = new DynamicJsonDocument(256);

            JsonArray msgArray = (*transferData.serializedData).createNestedArray("items");
            msgArray[0]["key"] = "Message";
            msgArray[0]["value"] = 0;

            msgArray[1]["key"] = "Location";
            msgArray[1]["value"] = 1;

            msgArray[2]["key"] = "User Information";
            msgArray[2]["value"] = 2;

            (*transferData.serializedData)["prompt"] = "Save:";
        }
        else if (oldState == selectionState && transferData.serializedData != nullptr)
        {
            DynamicJsonDocument *doc = (DynamicJsonDocument *)transferData.serializedData;

            if (doc->containsKey("return"))
            {
                MessageBase *currentPointedMessage = LoraUtils::GetCurrentUnreadMessage();

                if (currentPointedMessage != nullptr
                && currentPointedMessage->GetInstanceMessageType() == MessagePing::MessageType())
                {
                    MessagePing *pingMsg = (MessagePing *)currentPointedMessage;
                    switch ((*doc)["return"].as<int>())
                    {
                        // Save Message
                        case 0:
                        {
                            LoraUtils::AddSavedMessage(pingMsg->status);
                            Display_Utils::printCenteredText("Message saved", true);
                            display->display();
                            vTaskDelay(pdMS_TO_TICKS(2000));
                            break;
                        }

                        // Save Location
                        case 1:
                        {
                            newState = saveLocationState;
                            stateStack.push(receivedMessagesState);
                            if (transferData.serializedData != nullptr)
                            {
                                delete transferData.serializedData;
                            }
                            
                            transferData.serializedData = new DynamicJsonDocument(256);

                            (*transferData.serializedData)["cfgVal"] = pingMsg->status;
                            (*transferData.serializedData)["maxLen"] = STATUS_LENGTH;
                            (*transferData.serializedData)["lat"] = pingMsg->lat;
                            (*transferData.serializedData)["lon"] = pingMsg->lng;
                            
                            break;
                        }

                        // Save User Information
                        case 2:
                        {
                            UserInfo userInfo;
                            std::string username(pingMsg->senderName);
                            userInfo.Name = username;
                            userInfo.UserID = pingMsg->sender;
                            LoraUtils::AddUserInfo(userInfo);
                            Display_Utils::printCenteredText("User info saved", true);
                            display->display();
                            vTaskDelay(pdMS_TO_TICKS(2000));
                            break;
                        }

                        default:
                        break;
                    }

                    delete currentPointedMessage;
                }
            }
        }

        if (newState == selectLocationState)
        {
            if (transferData.serializedData != nullptr)
            {
                delete transferData.serializedData;
                transferData.serializedData = nullptr;
            }
            
            NavigationUtils::UpdateGPS();

            transferData.serializedData = new DynamicJsonDocument(1024);
            (*transferData.serializedData).createNestedArray("locations");
            auto currLocObj = (*transferData.serializedData)["locations"].createNestedObject();
            currLocObj["name"] = CURR_LOCATION;
            currLocObj["lat"] = NavigationUtils::GetLocation().lat();
            currLocObj["lng"] = NavigationUtils::GetLocation().lng();
            
            NavigationUtils::SerializeSavedLocations(*transferData.serializedData);
            
        }


        newState->enterState(transferData);

        currentState = newState;

        if (transferData.serializedData != nullptr)
        {
            delete transferData.serializedData;
        }
    }

protected:
    ReceivedMessagesState *receivedMessagesState;
    Tracking_State *trackingState;
    Select_Message_State *selectMessageState;
    Select_Location_State *selectLocationState;
    SelectKeyValueState *selectionState;
    SaveLocationState *saveLocationState;

    uint64_t recipientID;
    bool useCurrLocation;
    double latitude;
    double longitude;
    std::string locName;

    void clearMessageInfo()
    {
        recipientID = 0;
        useCurrLocation = false;
        latitude = 0;
        longitude = 0;
        locName = "";
    }
};