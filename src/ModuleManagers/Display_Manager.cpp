#include "Display_Manager.h"

TickType_t Display_Manager::lastButtonPressTick = 0;

// Display_Manager *Display_Manager::instance = NULL;
OLED_Window *Display_Manager::currentWindow = NULL;
OLED_Window *Display_Manager::rootWindow = NULL;
std::map<uint32_t, callbackPointer> Display_Manager::callbackMap;
std::map<uint8_t, inputCallbackPointer> Display_Manager::inputCallbackMap;
// std::unordered_map<size_t, uint8_t> Display_Manager::inputMap;
Adafruit_SSD1306 Display_Manager::display = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire);
int Display_Manager::refreshTimerID;

int Display_Manager::buttonFlashAnimationID = -1;

Lock_State *Display_Manager::lockState = nullptr;
int Display_Manager::lockStateTimerID = -1;


uint8_t Display_Manager::displayCommandQueueStorage[DISPLAY_COMMAND_QUEUE_LENGTH * sizeof(DisplayCommandQueueItem)];
StaticQueue_t Display_Manager::displayCommandQueueBuffer;
// QueueHandle_t Display_Manager::displayCommandQueue = xQueueCreateStatic(1, sizeof(DisplayCommandQueueItem), displayCommandQueueStorage, &Display_Manager::displayCommandQueueBuffer);
QueueHandle_t Display_Manager::displayCommandQueue = nullptr;

void Display_Manager::init()
{
    // Display_Manager::instance = new Display_Manager();
    OLED_Window::display = &display;
    Window_State::display = &display;
    OLED_Content::display = &display;
    Display_Utils::setDisplay(&display);
    Display_Utils::setDisplayDimensions(display.width(), display.height());

    auto displayCmdQueueID = System_Utils::registerQueue(
        DISPLAY_COMMAND_QUEUE_LENGTH, 
        sizeof(DisplayCommandQueueItem), 
        displayCommandQueueStorage, 
        displayCommandQueueBuffer);

    if (displayCmdQueueID != -1) 
    {
        displayCommandQueue = System_Utils::getQueue(displayCmdQueueID);
    } 
    else
    {
        Serial.println("Unable to initialize display command queue");
    }

    Display_Utils::setDisplayCommandQueue(displayCommandQueue);

    // display = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.display();

    // Register refresh timer
    Display_Manager::refreshTimerID = System_Utils::registerTimer("Display Refresh", 10000, Display_Manager::refreshTimerCallback);
    #if DEBUG == 1
    if (Display_Manager::refreshTimerID == -1)
    {
        Serial.println("Display_Manager::init: Failed to register refresh timer");
    }
    else
    {
        Serial.printf("Display_Manager::init: Registered refresh timer with ID %d\n", Display_Manager::refreshTimerID);
    }
    #endif
    OLED_Content::setTimerID(Display_Manager::refreshTimerID);
    Display_Utils::setRefreshTimerID(refreshTimerID);

    Display_Manager::initializeCallbacks();
    Display_Manager::generateHomeWindow(0);
}

OLED_Window *Display_Manager::attachNewWindow()
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

void Display_Manager::attachNewWindow(OLED_Window *window)
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

void Display_Manager::processCommandQueue(void *taskParams)
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
                    System_Utils::disableInterruptsInvoke();

                    // Clear queue after disabling interrupts to debounce
                    xQueueReset(displayCommandQueue);

                    CallbackData *cbPtr = Display_Manager::currentWindow->getCallbackDataByInputID(input);
                    CallbackData callbackData;

                    if (cbPtr != nullptr)
                    {
                        callbackData = CallbackData(*cbPtr);
                    }

                    // Pulse input LED if it exists
                    Display_Utils::getInputRaised().Invoke(input);

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
                    System_Utils::enableInterruptsInvoke();
                }

                break;
            }

            case CommandType::CALLBACK_COMMAND:
            {
                System_Utils::disableInterruptsInvoke();
                processEventCallback(displayCommand.commandData.callbackCommand.resourceID, 0);
                System_Utils::enableInterruptsInvoke();
                currentWindow->drawWindow();
                break;
            }
            }

            // System_Utils::sendDisplayContents(&display);
            Serial.println();
            
        }
    }
}

void Display_Manager::initializeCallbacks()
{
#if DEBUG == 1
    Serial.println("Display_Manager::initializeCallbacks");
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
    registerCallback(ACTION_OPEN_SAVED_MESSAGES_WINDOW, openSavedMsg);
    registerCallback(ACTION_CALL_FUNCTIONAL_WINDOW_STATE, callFunctionalWindowState);
    registerCallback(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, returnFromFunctionWindowState);
    registerCallback(ACTION_SWITCH_WINDOW_STATE, switchWindowState);
    registerCallback(ACTION_SAVE_LOCATION_WINDOW, openSaveLocationWindow);
    registerCallback(ACTION_OPEN_OTA_WINDOW, openOTAWindow);

    #ifdef USE_BLE
    registerCallback(ACTION_INIT_BLE, initializeBle);
    #endif

    registerInputCallback(MESSAGE_RECEIVED, processMessageReceived);
    registerInputCallback(BUTTON_SOS, openSOS);
#if DEBUG == 1
    Serial.println("Display_Manager::initializeCallbacks: done");
#endif
}

// std::vector<uint8_t> Display_Manager::getInputsFromNotification(uint32_t notification)
// {
//     std::vector<uint8_t> inputs;
//     for (auto it : Display_Manager::inputMap)
//     {
//         if (notification & it.first)
//         {
//             inputs.push_back(it.second);
//         }
//     }
//     return inputs;
// }

// void Display_Manager::callFunctionWindowState(uint8_t inputID)
// {
//     if (currentWindow != nullptr)
//     {
//         currentWindow->callFunctionState(inputID);
//     }
// }

void Display_Manager::returnFromFunctionWindowState(uint8_t inputID)
{
    if (currentWindow != nullptr)
    {
        currentWindow->returnFromFunctionState(inputID);
    }
}

void Display_Manager::processEventCallback(uint32_t resourceID, uint8_t inputID)
{
#if DEBUG == 1
    Serial.println("Display_Manager::processEventCallback");
    Serial.print("resourceID: ");
    Serial.println(resourceID, HEX);
#endif

    if (Display_Manager::callbackMap.find(resourceID) != Display_Manager::callbackMap.end())
    {
        callbackPointer callback = Display_Manager::callbackMap[resourceID];
        callback(inputID);
    }
}

void Display_Manager::processInputCallback(uint8_t inputID)
{
    if (Display_Manager::inputCallbackMap.find(inputID) != Display_Manager::inputCallbackMap.end())
    {
        inputCallbackPointer callback = Display_Manager::inputCallbackMap[inputID];
        callback();
    }
}

void Display_Manager::registerCallback(uint32_t resourceID, callbackPointer callback)
{
    Display_Manager::callbackMap[resourceID] = callback;
}

void Display_Manager::registerInputCallback(uint8_t inputID, inputCallbackPointer callback)
{
    Display_Manager::inputCallbackMap[inputID] = callback;
}

// void Display_Manager::registerInput(uint32_t resourceID, uint8_t inputID)
// {
//     Display_Manager::inputMap[resourceID] = inputID;
// }

void Display_Manager::displayLowBatteryShutdownNotice()
{
    display.clearDisplay();
    display.setCursor(Display_Utils::centerTextHorizontal(11), Display_Utils::selectTextLine(2));
    display.println("Low Battery");
    display.setCursor(Display_Utils::centerTextHorizontal(13), Display_Utils::selectTextLine(3));
    display.println("Shutting Down");
    display.display();
}

void Display_Manager::goBack(uint8_t inputID)
{
    if (Display_Manager::currentWindow->getParentWindow() != NULL)
    {
        OLED_Window *temp = Display_Manager::currentWindow;
        Display_Manager::currentWindow = Display_Manager::currentWindow->getParentWindow();
        delete temp;
        if (Display_Manager::currentWindow->isPaused)
        {
            Display_Manager::currentWindow->Resume();
        }
        Display_Manager::currentWindow->drawWindow();
        
        LED_Manager::clearRing();
    }
}

void Display_Manager::select(uint8_t inputID)
{
    if (currentWindow->currentState != nullptr && currentWindow->currentState->renderContent != nullptr)
    {
        if (currentWindow->currentState->renderContent->type == ContentType::LIST)
        {
            OLED_Content_List *list = (OLED_Content_List *)currentWindow->currentState->renderContent;
#if DEBUG == 1
            // Serial.print("Display_Manager::select found resourceID: ");
            // Serial.println(list->getCurrentNode()->resourceID);
#endif
            processEventCallback(list->getCurrentNode()->resourceID, inputID);
        }
#if DEBUG == 1
        // else
        // {
        //     Serial.println("Display_Manager::select: Content type is not list");
        // }
#endif
    }
}

void Display_Manager::generateHomeWindow(uint8_t inputID)
{
#if DEBUG == 1
    Serial.println("Display_Manager::generateHomeWindow");
#endif
    OLED_Window *newWindow = new Home_Window();
#if DEBUG == 1
    Serial.println("Display_Manager::generateHomeWindow: created new window");
#endif
    Display_Manager::attachNewWindow(newWindow);

#if DEBUG == 1
    Serial.println("Display_Manager::generateHomeWindow: attached new window");
#endif
    currentWindow->drawWindow();
}

void Display_Manager::generatePingWindow(uint8_t inputID)
{
    MessageBase *msg;

    if (currentWindow->content != nullptr && currentWindow->content->type == ContentType::STATUS)
    {
        msg = ((Received_Messages_Content *)currentWindow->content)->getCurrentMessage();
        Ping_Window *w = new Ping_Window(currentWindow, msg);
        Display_Manager::attachNewWindow(w);
        w->drawWindow();
    }
}
/*
void Display_Manager::generatePingWindowBroadcast(void *arg)
{
    if (currentWindow->content != nullptr && currentWindow->content->type == ContentType::STATUS)
    {
        Ping_Window *w = new Ping_Window(currentWindow, nullptr);
        Display_Manager::attachNewWindow(w);
        w->drawWindow();
    }
} */

void Display_Manager::generateSettingsWindow(uint8_t inputID)
{
    Settings_Window *newWindow = new Settings_Window(currentWindow);
    Display_Manager::attachNewWindow(newWindow);
    // newWindow->drawWindow();
}

void Display_Manager::generateStatusesWindow(uint8_t inputID)
{
    Statuses_Window *window = new Statuses_Window(currentWindow);
    Display_Manager::attachNewWindow(window);

    // window->drawWindow();
}

void Display_Manager::generateMenuWindow(uint8_t inputID)
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

    #ifdef USE_BLE
    menuWindow->addMenuItem("Init Bluetooth", ACTION_INIT_BLE);
    #endif
    
    menuWindow->addMenuItem("OTA Update", ACTION_OPEN_OTA_WINDOW);

    Display_Manager::attachNewWindow(menuWindow);


    // OLED_Window *newWindow = Display_Manager::attachNewWindow();
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

void Display_Manager::generateCompassWindow(uint8_t inputID)
{
    Compass_Window *window = new Compass_Window(currentWindow);
    Display_Manager::attachNewWindow(window);

    // window->drawWindow();
}

void Display_Manager::generateGPSWindow(uint8_t inputID)
{
    GPS_Window *window = new GPS_Window(currentWindow);
    Display_Manager::attachNewWindow(window);
}

void Display_Manager::generateLoRaTestWindow(uint8_t inputID)
{
    OLED_Window *newWindow = new LoRa_Test_Window(currentWindow);
    Display_Manager::attachNewWindow(newWindow);

    currentWindow->drawWindow();
}

void Display_Manager::flashDefaultSettings(uint8_t inputID)
{
    display.clearDisplay();
    display.setCursor(Display_Utils::centerTextHorizontal(11), Display_Utils::centerTextVertical());
    display.println("Flashing...");
    display.display();
    Settings_Manager::flashSettings();
    LoraUtils::FlashDefaultMessages();
    NavigationUtils::FlashSampleLocations();
    rebootDevice(inputID);
}

void Display_Manager::rebootDevice(uint8_t inputID)
{
    display.clearDisplay();
    display.setCursor(Display_Utils::centerTextHorizontal(12), Display_Utils::centerTextVertical());
    display.println("Rebooting...");
    display.display();
    delay(3000);
    ESP.restart();
}

void Display_Manager::toggleFlashlight(uint8_t inputID)
{
    LED_Manager::toggleFlashlight();
}

void Display_Manager::shutdownDevice(uint8_t inputID)
{
    display.clearDisplay();
    display.setCursor(Display_Utils::centerTextHorizontal(12), Display_Utils::centerTextVertical());
    display.println("Shutting down...");
    display.display();
    LED_Manager::ledShutdownAnimation();
    digitalWrite(KEEP_ALIVE_PIN, LOW);
}

void Display_Manager::toggleSilentMode(uint8_t inputID)
{
    System_Utils::silentMode = !System_Utils::silentMode;
}

void Display_Manager::quickActionMenu(uint8_t inputID)
{
    Menu_Window *newWindow = new Menu_Window(currentWindow);

    newWindow->addMenuItem("All Messages", ACTION_GENERATE_STATUSES_WINDOW);
    newWindow->addMenuItem("Save Current Location", ACTION_SAVE_LOCATION_WINDOW);
    newWindow->addMenuItem("Flashlight", ACTION_TOGGLE_FLASHLIGHT);
    newWindow->addMenuItem("Silent Mode", ACTION_TOGGLE_SILENT_MODE);
    newWindow->addMenuItem("Shutdown", ACTION_SHUTDOWN_DEVICE);
    newWindow->addMenuItem("Reboot Device", ACTION_REBOOT_DEVICE);

    Display_Manager::attachNewWindow(newWindow);

    currentWindow->drawWindow();
}

void Display_Manager::openSOS()
{
    Lock_State *lock = new Lock_State();
    lock->assignInput(BUTTON_3, ACTION_BACK, "Back");

    std::vector<TextDrawData> textData;

    TextDrawData data1;
    data1.text = "Confirm SOS with";
    data1.format.horizontalAlignment = TextAlignmentHorizontal::ALIGN_CENTER_HORIZONTAL;
    data1.format.verticalAlignment = TextAlignmentVertical::TEXT_LINE;
    data1.format.line = 2;
    data1.format.distanceFrom = 0;
    textData.push_back(data1);

    TextDrawData data2;
    data2.text = "button sequence";
    data2.format.horizontalAlignment = TextAlignmentHorizontal::ALIGN_CENTER_HORIZONTAL;
    data2.format.verticalAlignment = TextAlignmentVertical::TEXT_LINE;
    data2.format.line = 3;
    data2.format.distanceFrom = 0;
    textData.push_back(data2);

    Text_Display_Content *txtContent = new Text_Display_Content(textData);
    lock->renderContent = txtContent;
    
    textData.clear();

    TextDrawData data3;
    data3.text = "Sending SOS...";
    data3.format.horizontalAlignment = TextAlignmentHorizontal::ALIGN_CENTER_HORIZONTAL;
    data3.format.verticalAlignment = TextAlignmentVertical::ALIGN_CENTER_VERTICAL;
    data3.format.distanceFrom = 0;
    textData.push_back(data3);

    Text_Display_Content *txtContent2 = new Text_Display_Content(textData);

    Repeat_Message_State *repeat = new Repeat_Message_State(txtContent2, 0, 15);
    SOS_Window *sosWindow = new SOS_Window(currentWindow, repeat, lock);

    Display_Manager::attachNewWindow(sosWindow);

    currentWindow->drawWindow();
}

void Display_Manager::openSavedMsg(uint8_t inputID)
{
    OLED_Window *newWindow = new Saved_Msg_Window(currentWindow);
    Display_Manager::attachNewWindow(newWindow);

    currentWindow->drawWindow();
    vTaskDelay(pdMS_TO_TICKS(200));
}

void Display_Manager::switchWindowState(uint8_t inputID)
{
    if (currentWindow != nullptr)
    {
        // currentWindow->currentState->processInput(inputID);
        currentWindow->switchWindowState(inputID);
    }
}

void Display_Manager::callFunctionalWindowState(uint8_t inputID)
{
    if (currentWindow != nullptr)
    {
        // currentWindow->currentState->processInput(inputID);
        currentWindow->callFunctionState(inputID);
    }
}

void Display_Manager::processMessageReceived()
{
    if (System_Utils::silentMode == false)
    {
        LED_Manager::buzzerNotification();
    }
}


void Display_Manager::openSaveLocationWindow(uint8_t inputID)
{
    Save_Location_Window *window = new Save_Location_Window(currentWindow);
    Display_Manager::attachNewWindow(window);
    window->drawWindow();
}

void Display_Manager::enableLockScreen(size_t timeoutMS)
{
    if (timeoutMS > 0) 
    {
        lockStateTimerID = System_Utils::registerTimer("Lock Screen", timeoutMS, Display_Manager::cbLockDevice);
    }

    lockState = new Lock_State();
}

void Display_Manager::cbLockDevice(xTimerHandle xTimer)
{
    Display_Manager::lockDevice(0);
}

void Display_Manager::lockDevice(uint8_t inputID)
{
    if (currentWindow != nullptr)
    {
        currentWindow->callFunctionState(lockState);
    }
}

void Display_Manager::openOTAWindow(uint8_t inputID)
{
    OTA_Update_Window *window = new OTA_Update_Window(currentWindow);
    Display_Manager::attachNewWindow(window);
    window->drawWindow();
}

#ifdef USE_BLE
void Display_Manager::initializeBle(uint8_t inputID)
{
    System_Utils::initBluetooth();
}
#endif