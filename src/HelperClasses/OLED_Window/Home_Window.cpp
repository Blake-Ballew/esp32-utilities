#include "Home_Window.h"

Home_Window::Home_Window(OLED_Window *parent) : OLED_Window(parent)
{
    Home_Window();
}

Home_Window::Home_Window() : OLED_Window()
{    
    homeContent = new Home_Content(display);
    messageDisplay = new LoraMessageDisplay();

    unreadMessageState = new UnreadMessageState(messageDisplay);
    homeState = new Home_State(homeContent);
    trackingState = new Tracking_State();
    selectMessageState = new Select_Message_State();
    selectLocationState = new Select_Location_State();
    selectionState = new SelectKeyValueState();
    saveLocationState = new SaveLocationState();
    lockState = new Lock_State();

    contentList.push_back(homeContent);
    contentList.push_back(messageDisplay);
    contentList.push_back(saveLocationState->renderContent);

    stateList.push_back(homeState);
    stateList.push_back(trackingState);
    stateList.push_back(unreadMessageState);
    stateList.push_back(selectMessageState);
    stateList.push_back(selectLocationState);
    stateList.push_back(selectionState);
    stateList.push_back(saveLocationState);

    setInitialState(homeState);

    homeState->setAdjacentState(BUTTON_2, selectLocationState);
    homeState->setAdjacentState(BUTTON_3, lockState);
    homeState->setAdjacentState(ENC_DOWN, unreadMessageState);

    unreadMessageState->setAdjacentState(BUTTON_4, trackingState);
    unreadMessageState->setAdjacentState(BUTTON_2, selectLocationState);
    unreadMessageState->setAdjacentState(BUTTON_1, selectionState);

    unreadMessageState->assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Track");
    unreadMessageState->assignInput(BUTTON_2, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Reply");
    unreadMessageState->assignInput(BUTTON_3, ACTION_DEFER_CALLBACK_TO_WINDOW, "Mark Read");
    unreadMessageState->assignInput(BUTTON_1, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Save");

    selectLocationState->setAdjacentState(BUTTON_4, selectMessageState);
    selectLocationState->assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Select");

    trackingState->assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
}

void Home_Window::drawWindow()
{
    OLED_Window::drawWindow();

    if (LoraUtils::GetNumUnreadMessages() > 0) 
    {
        homeState->assignInput(ENC_DOWN, ACTION_CALL_FUNCTIONAL_WINDOW_STATE);
    }
    else
    {
        homeState->assignInput(ENC_DOWN, ACTION_NONE);
    }
}

void Home_Window::transferState(State_Transfer_Data &transferData)
{
#if DEBUG == 1
    Serial.println("Home_Window::transferState()");
#endif

    Window_State *oldState = transferData.oldState;
    Window_State *newState = transferData.newState;

    oldState->exitState(transferData);


    // Sending broadcast from home screen
    if (oldState == homeState && newState == selectLocationState)
    {
        sendDirect = false;
        recipientID = 0;
    }
    // Replying to a message
    else if (oldState == unreadMessageState && newState == selectLocationState)
    {
        auto *doc = transferData.serializedData;
        sendDirect = true;
        recipientID = (*doc)["recipientID"].as<uint32_t>();
    }
    // Location selected and moving to select message state
    else if (oldState == selectLocationState && transferData.serializedData != nullptr)
    {
#if DEBUG == 1
        Serial.println("Detecting return from select location state");
#endif
        DynamicJsonDocument *doc = (DynamicJsonDocument *)transferData.serializedData;

        if (doc->containsKey("name")
        && doc->containsKey("lat")
        && doc->containsKey("lng"))
        {
            locName = (*doc)["name"].as<std::string>();
            latitude = (*doc)["lat"].as<double>();
            longitude = (*doc)["lng"].as<double>();
            
            std::string currLocCompare = std::string(CURR_LOC);
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
    // Message selected. Time to send it
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
                LoraUtils::UserName().c_str(),
                0,
                LED_Utils::ThemeColor().r,
                LED_Utils::ThemeColor().g,
                LED_Utils::ThemeColor().b,
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
            newState = homeState;
        }
    }
    else if (oldState == unreadMessageState && newState == selectionState)
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
                        stateStack.push(unreadMessageState);
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
        currLocObj["name"] = CURR_LOC;
        currLocObj["lat"] = NavigationUtils::GetLocation().lat();
        currLocObj["lng"] = NavigationUtils::GetLocation().lng();
        
        NavigationUtils::SerializeSavedLocations(*transferData.serializedData);
        
    }
    // ***************** End mid-transfer logic *****************

    newState->enterState(transferData);

    currentState = newState;

    if (transferData.serializedData != nullptr)
    {
        delete transferData.serializedData;
    }
}

void Home_Window::clearMessageInfo()
{
    sendDirect = false;
    recipientID = 0;
    useCurrLocation = false;
    locationIdx = -1;
    locName.clear();
    latitude = 0;
    longitude = 0;
}