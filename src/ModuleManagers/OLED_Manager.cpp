#include "OLED_Manager.h"

TickType_t OLED_Manager::lastButtonPressTick = 0;

// OLED_Manager *OLED_Manager::instance = NULL;
OLED_Window *OLED_Manager::currentWindow = NULL;
OLED_Window *OLED_Manager::rootWindow = NULL;
std::map<uint32_t, callbackPointer> OLED_Manager::callbackMap;
std::map<uint8_t, inputCallbackPointer> OLED_Manager::inputCallbackMap;
std::unordered_map<size_t, uint8_t> OLED_Manager::inputMap;
Adafruit_SSD1306 OLED_Manager::display = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire);
int OLED_Manager::refreshTimerID;

uint8_t OLED_Manager::displayCommandQueueStorage[DISPLAY_COMMAND_QUEUE_LENGTH * sizeof(DisplayCommandQueueItem)];
StaticQueue_t OLED_Manager::displayCommandQueueBuffer;
QueueHandle_t OLED_Manager::displayCommandQueue = xQueueCreateStatic(1, sizeof(DisplayCommandQueueItem), displayCommandQueueStorage, &OLED_Manager::displayCommandQueueBuffer);

void OLED_Manager::init()
{
    // OLED_Manager::instance = new OLED_Manager();
    OLED_Window::display = &display;
    Window_State::display = &display;
    OLED_Content::display = &display;
    OLED_Content::displayCommandQueue = OLED_Manager::displayCommandQueue;
    // display = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.display();

    // Register refresh timer
    OLED_Manager::refreshTimerID = System_Utils::registerTimer("Display Refresh", 10000, OLED_Manager::refreshTimerCallback);
    if (OLED_Manager::refreshTimerID == -1)
    {
        Serial.println("OLED_Manager::init: Failed to register refresh timer");
    }
    OLED_Content::setTimerID(OLED_Manager::refreshTimerID);

    OLED_Manager::initializeCallbacks();
    OLED_Manager::generateHomeWindow(0);
}

OLED_Window *OLED_Manager::attachNewWindow()
{
    OLED_Window *newWindow;
    if (currentWindow == NULL)
    {
        newWindow = new OLED_Window();
        currentWindow = newWindow;
        rootWindow = newWindow;
    }
    else
    {
        newWindow = new OLED_Window(currentWindow);
        currentWindow->Pause();
        currentWindow = newWindow;
    }
    return newWindow;
}

void OLED_Manager::attachNewWindow(OLED_Window *window)
{
    if (currentWindow != nullptr)
        currentWindow->Pause();
    if (currentWindow == nullptr)
    {
        currentWindow = window;
        rootWindow = window;
    }
    else
    {
        currentWindow = window;
    }
}

void OLED_Manager::processCommandQueue(void *taskParams)
{
    while (true)
    {
        // xTaskNotifyWait(ULONG_MAX, ULONG_MAX, &notification, portMAX_DELAY);
        DisplayCommandQueueItem displayCommand;
        auto queueItemReceived = xQueueReceive(displayCommandQueue, &displayCommand, portMAX_DELAY);
        if (queueItemReceived == pdTRUE)
        {
            switch (displayCommand.commandType)
            {
            case CommandType::INPUT_COMMAND:
            {
                if ((xTaskGetTickCount() - lastButtonPressTick) > DEBOUNCE_DELAY)
                {
                    uint8_t input = displayCommand.commandData.inputCommand.inputID;
                    disableInterrupts();

                    // Clear queue after disabling interrupts to debounce
                    xQueueReset(displayCommandQueue);

                    CallbackData *cbPtr = OLED_Manager::currentWindow->getCallbackDataByInputID(input);
                    CallbackData callbackData;

                    if (cbPtr != nullptr)
                    {
                        callbackData = CallbackData(*cbPtr);
                    }

                    // Pulse LED if it exists
                    LED_Manager::pulseButton(input);

                    // Pass input to current window
                    if (currentWindow != nullptr)
                    {
                        currentWindow->execBtnCallback(input);
                    }

                    // Process input callback
                    processInputCallback(input);

                    // If callback data exists, execute callback
                    if (cbPtr != nullptr)
                    {
                        processEventCallback(callbackData.callbackID, input);
                    }

                    currentWindow->drawWindow();

                    lastButtonPressTick = xTaskGetTickCount();
                    enableInterrupts();
                }

                break;
            }

            case CommandType::CALLBACK_COMMAND:
            {
                disableInterrupts();
                processEventCallback(displayCommand.commandData.callbackCommand.resourceID, 0);
                enableInterrupts();
                currentWindow->drawWindow();
                break;
            }
            }

            // System_Utils::sendDisplayContents(&display);
            Serial.println();
            
        }

        /*         if (notification != 0 && (xTaskGetTickCount() - lastButtonPressTick) > DEBOUNCE_DELAY)
                {
        #if DEBUG == 1
                    Serial.println("OLED_Manager::processButtonPressEvent");
                    Serial.print("id: 0x");
                    Serial.println(notification, HEX);
        #endif
                    disableInterrupts();

                    // Clear queue after disabling interrupts to debounce
                    xQueueReset(displayCommandQueue);

                    auto inputs = getInputsFromNotification(notification);

                    for (auto input : inputs)
                    {
                        // Get callback data from the current window state if it exists
                        CallbackData *callbackData = OLED_Manager::currentWindow->getCallbackDataByInputID(input);

                        // Pulse LED if it exists
                        LED_Manager::pulseButton(input);

                        // Pass input to current window
                        if (currentWindow != nullptr)
                        {
                            currentWindow->execBtnCallback(input);
                        }

                        // Process input callback
                        processInputCallback(input);

                        // If callback data exists, execute callback
                        if (callbackData != nullptr)
                        {
                            processEventCallback(callbackData->callbackID, input);
                        }
                    }

                    currentWindow->drawWindow();

                    // reset notification
                    notification = 0;
                    lastButtonPressTick = xTaskGetTickCount();
                    enableInterrupts();
        }*/
    }
}

void OLED_Manager::initializeCallbacks()
{
#if DEBUG == 1
    Serial.println("OLED_Manager::initializeCallbacks");
#endif
    registerCallback(ACTION_BACK, goBack);
    registerCallback(ACTION_SELECT, select);
    registerCallback(ACTION_GENERATE_HOME_WINDOW, generateHomeWindow);
    registerCallback(ACTION_GENERATE_SETTINGS_WINDOW, generateSettingsWindow);
    registerCallback(ACTION_GENERATE_STATUSES_WINDOW, generateStatusesWindow);
    registerCallback(ACTION_GENERATE_MENU_WINDOW, generateMenuWindow);
    registerCallback(ACTION_GENERATE_COMPASS_WINDOW, generateCompassWindow);
    registerCallback(ACTION_GENERATE_GPS_WINDOW, generateGPSWindow);
    registerCallback(ACTION_GENERATE_LORA_TEST_WINDOW, generateLoRaTestWindow);
    registerCallback(ACTION_SEND_PING, generatePingWindow);
    registerCallback(ACTION_FLASH_DEFAULT_SETTINGS, flashDefaultSettings);
    registerCallback(ACTION_REBOOT_DEVICE, rebootDevice);
    registerCallback(ACTION_TOGGLE_FLASHLIGHT, toggleFlashlight);
    registerCallback(ACTION_SHUTDOWN_DEVICE, shutdownDevice);
    registerCallback(ACTION_TOGGLE_SILENT_MODE, toggleSilentMode);
    registerCallback(ACTION_GENERATE_QUICK_ACTION_MENU, quickActionMenu);
    registerCallback(ACTION_SOS, openSOS);
    registerCallback(ACTION_OPEN_SAVED_MESSAGES_WINDOW, openSavedMsg);
    registerCallback(ACTION_CALL_FUNCTIONAL_WINDOW_STATE, callFunctionalWindowState);
    registerCallback(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, returnFromFunctionWindowState);
    registerCallback(ACTION_SWITCH_WINDOW_STATE, switchWindowState);
    registerCallback(ACTION_SAVE_LOCATION_WINDOW, openSaveLocationWindow);

    registerInputCallback(MESSAGE_RECEIVED, processMessageReceived);
#if DEBUG == 1
    Serial.println("OLED_Manager::initializeCallbacks: done");
#endif
}

std::vector<uint8_t> OLED_Manager::getInputsFromNotification(uint32_t notification)
{
    std::vector<uint8_t> inputs;
    for (auto it : OLED_Manager::inputMap)
    {
        if (notification & it.first)
        {
            inputs.push_back(it.second);
        }
    }
    return inputs;
}

// void OLED_Manager::callFunctionWindowState(uint8_t inputID)
// {
//     if (currentWindow != nullptr)
//     {
//         currentWindow->callFunctionState(inputID);
//     }
// }

void OLED_Manager::returnFromFunctionWindowState(uint8_t inputID)
{
    if (currentWindow != nullptr)
    {
        currentWindow->returnFromFunctionState(inputID);
    }
}

void OLED_Manager::processEventCallback(uint32_t resourceID, uint8_t inputID)
{
#if DEBUG == 1
    Serial.println("OLED_Manager::processEventCallback");
    Serial.print("resourceID: ");
    Serial.println(resourceID, HEX);
#endif

    if (OLED_Manager::callbackMap.find(resourceID) != OLED_Manager::callbackMap.end())
    {
        callbackPointer callback = OLED_Manager::callbackMap[resourceID];
        callback(inputID);
    }
}

void OLED_Manager::processInputCallback(uint8_t inputID)
{
    if (OLED_Manager::inputCallbackMap.find(inputID) != OLED_Manager::inputCallbackMap.end())
    {
        inputCallbackPointer callback = OLED_Manager::inputCallbackMap[inputID];
        callback();
    }
}

void OLED_Manager::registerCallback(uint32_t resourceID, callbackPointer callback)
{
    OLED_Manager::callbackMap[resourceID] = callback;
}

void OLED_Manager::registerInputCallback(uint8_t inputID, inputCallbackPointer callback)
{
    OLED_Manager::inputCallbackMap[inputID] = callback;
}

void OLED_Manager::registerInput(uint32_t resourceID, uint8_t inputID)
{
    OLED_Manager::inputMap[resourceID] = inputID;
}

void OLED_Manager::displayLowBatteryShutdownNotice()
{
    display.clearDisplay();
    display.setCursor(OLED_Content::centerTextHorizontal(11), OLED_Content::selectTextLine(2));
    display.println("Low Battery");
    display.setCursor(OLED_Content::centerTextHorizontal(13), OLED_Content::selectTextLine(3));
    display.println("Shutting Down");
    display.display();
}

void OLED_Manager::goBack(uint8_t inputID)
{
    if (OLED_Manager::currentWindow->getParentWindow() != NULL)
    {
        if (currentWindow->stateStack.size() > 0)
        {
            currentWindow->returnFromFunctionState(inputID);
        }
        else
        {
            OLED_Window *temp = OLED_Manager::currentWindow;
            OLED_Manager::currentWindow = OLED_Manager::currentWindow->getParentWindow();
            delete temp;
            if (OLED_Manager::currentWindow->isPaused)
            {
                OLED_Manager::currentWindow->Resume();
            }
            OLED_Manager::currentWindow->drawWindow();
        }
        LED_Manager::clearRing();
    }
}

void OLED_Manager::select(uint8_t inputID)
{
    if (currentWindow->currentState != nullptr && currentWindow->currentState->renderContent != nullptr)
    {
        if (currentWindow->currentState->renderContent->type == ContentType::LIST)
        {
            OLED_Content_List *list = (OLED_Content_List *)currentWindow->currentState->renderContent;
#if DEBUG == 1
            Serial.print("OLED_Manager::select found resourceID: ");
            Serial.println(list->getCurrentNode()->resourceID);
#endif
            processEventCallback(list->getCurrentNode()->resourceID, inputID);
        }
#if DEBUG == 1
        else
        {
            Serial.println("OLED_Manager::select: Content type is not list");
        }
#endif
    }
}

void OLED_Manager::generateHomeWindow(uint8_t inputID)
{
#if DEBUG == 1
    Serial.println("OLED_Manager::generateHomeWindow");
#endif
    OLED_Window *newWindow = new Home_Window();
#if DEBUG == 1
    Serial.println("OLED_Manager::generateHomeWindow: created new window");
#endif
    OLED_Manager::attachNewWindow(newWindow);

#if DEBUG == 1
    Serial.println("OLED_Manager::generateHomeWindow: attached new window");
#endif
    currentWindow->drawWindow();
}

void OLED_Manager::generatePingWindow(uint8_t inputID)
{
    Message_Base *msg;

    if (currentWindow->content != nullptr && currentWindow->content->type == ContentType::STATUS)
    {
        msg = ((Received_Messages_Content *)currentWindow->content)->getCurrentMessage();
        Ping_Window *w = new Ping_Window(currentWindow, msg);
        OLED_Manager::attachNewWindow(w);
        w->drawWindow();
    }
}
/*
void OLED_Manager::generatePingWindowBroadcast(void *arg)
{
    if (currentWindow->content != nullptr && currentWindow->content->type == ContentType::STATUS)
    {
        Ping_Window *w = new Ping_Window(currentWindow, nullptr);
        OLED_Manager::attachNewWindow(w);
        w->drawWindow();
    }
} */

void OLED_Manager::generateSettingsWindow(uint8_t inputID)
{
    Settings_Window *newWindow = new Settings_Window(currentWindow);
    OLED_Manager::attachNewWindow(newWindow);
    // newWindow->drawWindow();
}

void OLED_Manager::generateStatusesWindow(uint8_t inputID)
{
    Statuses_Window *window = new Statuses_Window(currentWindow);
    OLED_Manager::attachNewWindow(window);

    // window->drawWindow();
}

void OLED_Manager::generateMenuWindow(uint8_t inputID)
{
    Menu_Window *menuWindow = new Menu_Window(currentWindow);

    menuWindow->addMenuItem("Messages", ACTION_GENERATE_STATUSES_WINDOW);
    menuWindow->addMenuItem("Saved Messages", ACTION_OPEN_SAVED_MESSAGES_WINDOW);
    menuWindow->addMenuItem("Settings", ACTION_GENERATE_SETTINGS_WINDOW);
    menuWindow->addMenuItem("Flashlight", ACTION_TOGGLE_FLASHLIGHT);
    menuWindow->addMenuItem("Compass", ACTION_GENERATE_COMPASS_WINDOW);
    menuWindow->addMenuItem("GPS", ACTION_GENERATE_GPS_WINDOW);
    // menuWindow->addMenuItem("LoRa Test", ACTION_GENERATE_LORA_TEST_WINDOW);
    menuWindow->addMenuItem("Flash Settings", ACTION_FLASH_DEFAULT_SETTINGS);
    menuWindow->addMenuItem("Reboot Device", ACTION_REBOOT_DEVICE);
    menuWindow->addMenuItem("Shutdown Device", ACTION_SHUTDOWN_DEVICE);

    OLED_Manager::attachNewWindow(menuWindow);


    // OLED_Window *newWindow = OLED_Manager::attachNewWindow();
    // Select_Content_List_State *state = new Select_Content_List_State();
    // newWindow->currentState = state;

    // state->assignInput(BUTTON_3, ACTION_BACK, "Back");
    // state->assignInput(BUTTON_4, ACTION_SELECT, "Select");

    // OLED_Content_List *list = new OLED_Content_List(&display);
    // state->renderContent = list;

    // list->addNode(new Content_Node(ACTION_GENERATE_STATUSES_WINDOW, "Messages", 8));
    // list->addNode(new Content_Node(ACTION_OPEN_SAVED_MESSAGES_WINDOW, "Saved Messages", 15));
    // list->addNode(new Content_Node(ACTION_GENERATE_SETTINGS_WINDOW, "Settings", 8));
    // list->addNode(new Content_Node(ACTION_TOGGLE_FLASHLIGHT, "Flashlight", 10));
    // list->addNode(new Content_Node(ACTION_GENERATE_COMPASS_WINDOW, "Compass", 7));
    // list->addNode(new Content_Node(ACTION_GENERATE_GPS_WINDOW, "GPS", 3));
    // // list->addNode(new Content_Node(ACTION_GENERATE_LORA_TEST_WINDOW, "LoRa Test", 9));
    // list->addNode(new Content_Node(ACTION_FLASH_DEFAULT_SETTINGS, "Flash Settings", 15));
    // list->addNode(new Content_Node(ACTION_REBOOT_DEVICE, "Reboot Device", 14));
    // list->addNode(new Content_Node(ACTION_SHUTDOWN_DEVICE, "Shutdown Device", 15));

    currentWindow->drawWindow();
}

void OLED_Manager::generateCompassWindow(uint8_t inputID)
{
    Compass_Window *window = new Compass_Window(currentWindow);
    OLED_Manager::attachNewWindow(window);

    // window->drawWindow();
}

void OLED_Manager::generateGPSWindow(uint8_t inputID)
{
    GPS_Window *window = new GPS_Window(currentWindow);
    OLED_Manager::attachNewWindow(window);
}

void OLED_Manager::generateLoRaTestWindow(uint8_t inputID)
{
    OLED_Window *newWindow = new LoRa_Test_Window(currentWindow);
    OLED_Manager::attachNewWindow(newWindow);

    currentWindow->drawWindow();
}

void OLED_Manager::flashDefaultSettings(uint8_t inputID)
{
    display.clearDisplay();
    display.setCursor(OLED_Content::centerTextHorizontal(11), OLED_Content::centerTextVertical());
    display.println("Flashing...");
    display.display();
    Settings_Manager::flashSettings();
    rebootDevice(inputID);
}

void OLED_Manager::rebootDevice(uint8_t inputID)
{
    display.clearDisplay();
    display.setCursor(OLED_Content::centerTextHorizontal(12), OLED_Content::centerTextVertical());
    display.println("Rebooting...");
    display.display();
    delay(3000);
    ESP.restart();
}

void OLED_Manager::toggleFlashlight(uint8_t inputID)
{
    LED_Manager::toggleFlashlight();
}

void OLED_Manager::shutdownDevice(uint8_t inputID)
{
    display.clearDisplay();
    display.setCursor(OLED_Content::centerTextHorizontal(12), OLED_Content::centerTextVertical());
    display.println("Shutting down...");
    display.display();
    LED_Manager::ledShutdownAnimation();
    digitalWrite(KEEP_ALIVE_PIN, LOW);
}

void OLED_Manager::toggleSilentMode(uint8_t inputID)
{
    System_Utils::silentMode = !System_Utils::silentMode;
}

void OLED_Manager::quickActionMenu(uint8_t inputID)
{
    Menu_Window *newWindow = new Menu_Window(currentWindow);

    newWindow->addMenuItem("All Messages", ACTION_GENERATE_STATUSES_WINDOW);
    newWindow->addMenuItem("Save Current Location", ACTION_SAVE_LOCATION_WINDOW);
    newWindow->addMenuItem("Flashlight", ACTION_TOGGLE_FLASHLIGHT);
    newWindow->addMenuItem("Silent Mode", ACTION_TOGGLE_SILENT_MODE);
    newWindow->addMenuItem("Shutdown", ACTION_SHUTDOWN_DEVICE);
    newWindow->addMenuItem("Reboot Device", ACTION_REBOOT_DEVICE);

    OLED_Manager::attachNewWindow(newWindow);

    currentWindow->drawWindow();
}

void OLED_Manager::openSOS(uint8_t inputID)
{
    if (currentWindow->content != nullptr && currentWindow->content->type == ContentType::SOS)
    {
        return;
    }
    OLED_Window *sosWindow = new SOS_Window(currentWindow);
    OLED_Manager::attachNewWindow(sosWindow);
    sosWindow->drawWindow();
}

void OLED_Manager::openSavedMsg(uint8_t inputID)
{
    OLED_Window *newWindow = new Saved_Msg_Window(currentWindow);
    OLED_Manager::attachNewWindow(newWindow);

    currentWindow->drawWindow();
    vTaskDelay(pdMS_TO_TICKS(200));
}

void OLED_Manager::switchWindowState(uint8_t inputID)
{
    if (currentWindow != nullptr)
    {
        // currentWindow->currentState->processInput(inputID);
        currentWindow->switchWindowState(inputID);
    }
}

void OLED_Manager::callFunctionalWindowState(uint8_t inputID)
{
    if (currentWindow != nullptr)
    {
        // currentWindow->currentState->processInput(inputID);
        currentWindow->callFunctionState(inputID);
    }
}

void OLED_Manager::processMessageReceived()
{
    if (System_Utils::silentMode == false)
    {
        LED_Manager::buzzerNotification();
    }
}


void OLED_Manager::openSaveLocationWindow(uint8_t inputID)
{
    Save_Location_Window *window = new Save_Location_Window(currentWindow);
    OLED_Manager::attachNewWindow(window);
    window->drawWindow();
}