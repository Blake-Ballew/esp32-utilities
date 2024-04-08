#include "Home_Window.h"

Home_Window::Home_Window(OLED_Window *parent) : OLED_Window(parent)
{
    Home_Window();
}

Home_Window::Home_Window() : OLED_Window()
{
#if DEBUG == 1
    Serial.println("DOES PRINTING EVEN WORK HERE??????????????");
#endif
    homeContent = new Home_Content(display);
    trackingContent = new Tracking_Content(display);
    receivedMessagesContent = new Received_Messages_Content(display, false, false);
    savedMessagesContent = new Saved_Messages_Content();
    savedLocationsContent = new Saved_Locations_Content(display);

    receivedMessagesState = new Received_Messages_State(receivedMessagesContent);
    homeState = new Home_State(homeContent);
    trackingState = new Tracking_State(trackingContent);
    selectMessageState = new Select_Message_State(savedMessagesContent);
    selectLocationState = new Select_Location_State(savedLocationsContent);

    contentList.push_back(homeContent);
    contentList.push_back(trackingContent);
    contentList.push_back(receivedMessagesContent);
    contentList.push_back(selectMessageState->renderContent);
    contentList.push_back(selectLocationState->renderContent);

    stateList.push_back(homeState);
    stateList.push_back(trackingState);
    stateList.push_back(receivedMessagesState);
    stateList.push_back(selectMessageState);
    stateList.push_back(selectLocationState);

    setInitialState(homeState);

    homeState->setAdjacentState(BUTTON_2, selectLocationState);
    homeState->setAdjacentState(ENC_DOWN, receivedMessagesState);

    receivedMessagesState->setAdjacentState(ENC_UP, homeState);
    receivedMessagesState->setAdjacentState(BUTTON_1, trackingState);
    receivedMessagesState->setAdjacentState(BUTTON_4, selectLocationState);

    System_Utils::monitorSystemHealth(nullptr);
}

// void Home_Window::Pause()
// {
//     homeContent->Pause();
// }

// void Home_Window::Resume()
// {
//     homeContent->Resume();
// }

/* void Home_Window::encUp()
{
    content->encUp();

    if (content->type == ContentType::HOME)
    {
        // Change button actions based on content mode
        if (homeContent->contentMode == 1)
        {
            homeContent1();
        }
        else if (homeContent->contentMode == 2)
        {
            homeContent2();
        }
    }
}

void Home_Window::encDown()
{
    content->encDown();

    if (content->type == ContentType::HOME)
    {
        // Change button actions based on content mode
        if (homeContent->contentMode == 1)
        {
            homeContent1();
        }
        else if (homeContent->contentMode == 2)
        {
            homeContent2();
        }
    }
} */

/* void Home_Window::drawWindow()
{
#if DEBUG == 1
    Serial.println("Home_Window::drawWindow()");
#endif
    OLED_Window::drawWindow();
    // homeContent->printContent();
} */

/* void Home_Window::execBtnCallback(uint8_t inputID)
{
    switch (inputID)
    {
    case BUTTON_1:
    {
        if (content->type == ContentType::HOME)
        {
            if (homeContent->contentMode == Home_Content::HOME_CONTENT_MESSAGES)
            {
                swapToTracking(homeContent->getCurrentMessage());
            }
        }
    }
    break;
    case BUTTON_2:
    {
        if (content->type == ContentType::HOME)
        {
            if (homeContent->contentMode == Home_Content::HOME_CONTENT_MAIN)
            {
                swapToPing(nullptr);
            }
            else if (homeContent->contentMode == Home_Content::HOME_CONTENT_MESSAGES)
            {
                // Mark message read

                Message_Base *msg = homeContent->getCurrentMessage();
                if (msg != nullptr)
                {
                    msg->messageOpened = true;
                    encUp();
                }
            }
        }
    }
    break;
    case BUTTON_3:
    {
        if (content->type == ContentType::PING)
        {
            swapToHome();
        }

        if (content->type == ContentType::TRACKING)
        {
            swapToHome();
        }
    }
    break;
    case BUTTON_4:
    {
        if (content->type == ContentType::PING)
        {
#if DEBUG == 1
            Serial.println("Sending ping");
#endif
            pingContent->sendPing();
#if DEBUG == 1
            Serial.println("Ping sent");
#endif
            pingContent->unassignMsg();
            swapToHome();
        }

        if (content->type == ContentType::HOME)
        {
            if (homeContent->contentMode == Home_Content::HOME_CONTENT_MESSAGES)
            {
                Message_Base *msg = homeContent->getCurrentMessage();
                if (msg != nullptr)
                {
                    swapToPing(msg);
                }
            }
        }
    }
    break;
    default:
        break;
    }
} */

void Home_Window::transferState(State_Transfer_Data &transferData)
{
#if DEBUG == 1
    Serial.println("Home_Window::transferState()");
#endif

    Window_State *oldState = transferData.oldState;
    Window_State *newState = transferData.newState;

    oldState->exitState(transferData);

    // ***************** Mid-transfer logic *****************
    if ((oldState == homeState || oldState == receivedMessagesState) && newState == selectLocationState)
    {
#if DEBUG == 1
        Serial.println("Transfering to select location state");
#endif
        if (transferData.serializedData != nullptr)
        {
            DynamicJsonDocument *doc = transferData.serializedData;
            if (doc->containsKey("sendDirect"))
            {
                sendDirect = (*doc)["sendDirect"].as<bool>();
                if (sendDirect)
                {
                    recipientID = (*doc)["recipientID"].as<uint64_t>();
                }
                else
                {
                    recipientID = 0;
                }
            }
        }
    }
    else if (oldState == selectLocationState && transferData.serializedData != nullptr)
    {
#if DEBUG == 1
        Serial.println("Detecting return from select location state");
#endif
        DynamicJsonDocument *doc = (DynamicJsonDocument *)transferData.serializedData;
        if (doc != nullptr && doc->containsKey("isCurrLocation"))
        {
            if ((*doc)["isCurrLocation"] == true)
            {
#if DEBUG == 1
                Serial.println("Using current location");
#endif
                useCurrLocation = true;
            }
            else
            {
#if DEBUG == 1
                Serial.println("Using selected location");
#endif
                useCurrLocation = false;
                locationIdx = (*doc)["idx"].as<int>();
                locName = (*doc)["name"].as<const char *>();
                latitude = (*doc)["lat"].as<double>();
                longitude = (*doc)["lon"].as<double>();
            }
        }
#if DEBUG == 1
        else
        {
            Serial.println("No location data");
        }
#endif

        // Transfer to Select Message State
        delete transferData.serializedData;
        transferData.serializedData = nullptr;

        if (!useCurrLocation)
        {
            transferData.serializedData = new DynamicJsonDocument(128);
            (*transferData.serializedData)["message"] = locName;
        }

        newState = selectMessageState;
    }
    else if (oldState == selectMessageState && transferData.serializedData != nullptr)
    {
        DynamicJsonDocument *doc = (DynamicJsonDocument *)transferData.serializedData;
        if (doc != nullptr && doc->containsKey("message"))
        {
            const char *message = (*doc)["message"].as<const char *>();
            // Send message

            Message_Ping *newMsg = new Message_Ping(
                Navigation_Manager::getTime().value(),
                Navigation_Manager::getDate().value(),
                recipientID,
                Network_Manager::userID,
                Settings_Manager::settings["User"]["Name"]["cfgVal"].as<const char *>(),
                0,
                Settings_Manager::settings["User"]["Theme Red"]["cfgVal"].as<uint8_t>(),
                Settings_Manager::settings["User"]["Theme Green"]["cfgVal"].as<uint8_t>(),
                Settings_Manager::settings["User"]["Theme Blue"]["cfgVal"].as<uint8_t>(),
                latitude,
                longitude,
                message);

            uint8_t returnCode;

            if (sendDirect)
            {
                returnCode = Network_Manager::queueMessageToUser(recipientID, newMsg);
            }
            else
            {
                returnCode = Network_Manager::queueBroadcastMessage(newMsg);
            }

            OLED_Content::clearContentArea();
            auto returnStr = Network_Manager::getReturnCodeString(returnCode);

            display->setCursor(OLED_Content::centerTextHorizontal(returnStr), OLED_Content::centerTextVertical());
            display->print(returnStr);

            display->display();
            clearMessageInfo();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
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
    locName = nullptr;
    latitude = 0;
    longitude = 0;
}