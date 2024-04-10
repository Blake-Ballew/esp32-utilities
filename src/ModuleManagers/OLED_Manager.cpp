#include "OLED_Manager.h"

TickType_t OLED_Manager::lastButtonPressTick = 0;

OLED_Manager *OLED_Manager::instance = NULL;
OLED_Window *OLED_Manager::currentWindow = NULL;
OLED_Window *OLED_Manager::rootWindow = NULL;
std::map<uint32_t, callbackPointer> OLED_Manager::callbackMap;
std::map<uint8_t, inputCallbackPointer> OLED_Manager::inputCallbackMap;
std::unordered_map<size_t, uint8_t> OLED_Manager::inputMap;
Adafruit_SSD1306 OLED_Manager::display = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire);

void OLED_Manager::init()
{
    OLED_Manager::instance = new OLED_Manager();
    OLED_Window::display = &display;
    // display = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
#if DEBUG == 1
    Serial.print("OLED_Manager::display height: ");
    Serial.println(display.height());
    Serial.print("OLED_Manager::display width: ");
    Serial.println(display.width());
#endif
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.display();
    OLED_Manager::initializeCallbacks();
    OLED_Manager::generateHomeWindow(0);
    // esp_event_handler_register(loop_handle, EVENT_BUTTON_IO, ESP_EVENT_ANY_ID, OLED_Manager::processButtonPressEvent, NULL);
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

void OLED_Manager::processButtonPressEvent(void *taskParams)
{
    uint32_t notification;
    while (true)
    {
        xTaskNotifyWait(ULONG_MAX, ULONG_MAX, &notification, portMAX_DELAY);
        if (notification != 0 && (xTaskGetTickCount() - lastButtonPressTick) > DEBOUNCE_DELAY)
        {
#if DEBUG == 1
            Serial.println("OLED_Manager::processButtonPressEvent");
            Serial.print("id: 0x");
            Serial.println(notification, HEX);
#endif
            disableInterrupts();

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

            /*             if (OLED_Manager::currentWindow != NULL)
                        {
                            uint32_t callbackID = 0;
                            uint8_t inputID = 0;

                            if (notification & BIT_SHIFT((uint8_t)EVENT_BUTTON_1))
                            {

                                if (OLED_Manager::currentWindow->btn1CallbackID != 0)
                                {
                                    callbackID = OLED_Manager::currentWindow->btn1CallbackID;
                                    inputID = BUTTON_1;
                                    LED_Manager::pulseButton(1);
                                    // processEventCallback(OLED_Manager::currentWindow->btn1CallbackID, NULL);
                                    //  OLED_Manager::callbackMap[OLED_Manager::currentWindow->btn1CallbackID](event_data);
                                }
                            }
                            else if (notification & BIT_SHIFT((uint8_t)EVENT_BUTTON_2))
                            {
                                if (OLED_Manager::currentWindow->btn2CallbackID != 0)
                                {
                                    callbackID = OLED_Manager::currentWindow->btn2CallbackID;
                                    inputID = 2;
                                    LED_Manager::pulseButton(2);
                                    // processEventCallback(OLED_Manager::currentWindow->btn2CallbackID, NULL);
                                }
                            }
                            else if (notification & BIT_SHIFT((uint8_t)EVENT_BUTTON_3))
                            {
                                if (OLED_Manager::currentWindow->btn3CallbackID != 0)
                                {
                                    callbackID = OLED_Manager::currentWindow->btn3CallbackID;
                                    inputID = 3;
                                    LED_Manager::pulseButton(3);
                                    // processEventCallback(OLED_Manager::currentWindow->btn3CallbackID, NULL);
                                }
                            }
                            else if (notification & BIT_SHIFT((uint8_t)EVENT_BUTTON_4))
                            {
                                if (OLED_Manager::currentWindow->btn4CallbackID != 0)
                                {
                                    callbackID = OLED_Manager::currentWindow->btn4CallbackID;
                                    inputID = 4;
                                    LED_Manager::pulseButton(4);
                                    // processEventCallback(OLED_Manager::currentWindow->btn4CallbackID, NULL);
                                }
                            }
                            else if (notification & BIT_SHIFT((uint8_t)EVENT_ENCODER_UP))
                            {
                                OLED_Manager::currentWindow->encUp();
                                LED_Manager::pulseButton(6);
                                inputID = 5;
                            }
                            else if (notification & BIT_SHIFT((uint8_t)EVENT_ENCODER_DOWN))
                            {
                                OLED_Manager::currentWindow->encDown();
                                LED_Manager::pulseButton(7);
                                inputID = 6;
                            }
                            else if (notification & BIT_SHIFT((uint8_t)EVENT_MESSAGE_RECEIVED))
                            {
                                if (OLED_Manager::currentWindow != nullptr && OLED_Manager::currentWindow->content != nullptr)
                                {
                                    if (OLED_Manager::currentWindow->content->type == ContentType::LORA_TEST)
                                    {
                                        LoRa_Test_Content *c = (LoRa_Test_Content *)OLED_Manager::currentWindow->content;
                                        c->updateMessages();
                                    }
                                    else if (currentWindow->content->type == ContentType::STATUS)
                                    {
                                        Received_Messages_Content *c = (Received_Messages_Content *)currentWindow->content;
                                        c->updateMessages();
                                    }
                                }
                                if (System_Utils::silentMode == false)
                                {
                                    LED_Manager::buzzerNotification();
                                }
                            }

                            // If old callback system isn't used, check new system
                            if (callbackID == 0)
                            {
                                CallbackData *callbackData = OLED_Manager::currentWindow->getCallbackDataByInputID(inputID);
                                if (callbackData != nullptr)
                                {
                                    callbackID = callbackData->callbackID;
                                }
                            }

                            if (callbackID != 0)
                            {
                                OLED_Manager::currentWindow->execBtnCallback(inputID);
                                if (callbackID < 0xF0000000)
                                {
                                    processEventCallback(callbackID, inputID);
                                }
                                else if (callbackID == ACTION_SWITCH_WINDOW_STATE)
                                {
                                    OLED_Manager::currentWindow->switchWindowState(inputID);
                                }
                            } */

            currentWindow->drawWindow();

            // reset notification
            notification = 0;
            lastButtonPressTick = xTaskGetTickCount();
            enableInterrupts();
        }
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

void OLED_Manager::callFunctionWindowState(uint8_t inputID)
{
    if (currentWindow != nullptr)
    {
        currentWindow->callFunctionState(inputID);
    }
}

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
    if (OLED_Manager::currentWindow->content != NULL)
    {
        if (OLED_Manager::currentWindow->currentState->renderContent->type == ContentType::LIST)
        {
            OLED_Content_List *list = (OLED_Content_List *)currentWindow->content;
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
    OLED_Settings_Window *newWindow = new OLED_Settings_Window(currentWindow);
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
    OLED_Window *newWindow = OLED_Manager::attachNewWindow();
    Select_Content_List_State *state = new Select_Content_List_State();
    newWindow->currentState = state;

    state->assignInput(BUTTON_3, ACTION_BACK, "Back");
    state->assignInput(BUTTON_4, ACTION_SELECT, "Select");

    OLED_Content_List *list = new OLED_Content_List(&display);
    state->renderContent = list;

    list->addNode(new Content_Node(ACTION_GENERATE_STATUSES_WINDOW, "Messages", 8));
    list->addNode(new Content_Node(ACTION_OPEN_SAVED_MESSAGES_WINDOW, "Saved Messages", 15));
    list->addNode(new Content_Node(ACTION_GENERATE_SETTINGS_WINDOW, "Settings", 8));
    list->addNode(new Content_Node(ACTION_TOGGLE_FLASHLIGHT, "Flashlight", 10));
    list->addNode(new Content_Node(ACTION_GENERATE_COMPASS_WINDOW, "Compass", 7));
    list->addNode(new Content_Node(ACTION_GENERATE_GPS_WINDOW, "GPS", 3));
    // list->addNode(new Content_Node(ACTION_GENERATE_LORA_TEST_WINDOW, "LoRa Test", 9));
    list->addNode(new Content_Node(ACTION_FLASH_DEFAULT_SETTINGS, "Flash Settings", 15));
    list->addNode(new Content_Node(ACTION_REBOOT_DEVICE, "Reboot Device", 14));
    list->addNode(new Content_Node(ACTION_SHUTDOWN_DEVICE, "Shutdown Device", 15));

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
    OLED_Window *newWindow = OLED_Manager::attachNewWindow();

    Window_State *state = new Window_State();
    newWindow->currentState = state;

    state->assignInput(BUTTON_3, ACTION_BACK, "Back");
    state->assignInput(BUTTON_4, ACTION_SELECT, "Select");

    OLED_Content_List *list = new OLED_Content_List(&display);

    state->renderContent = list;

    list->addNode(new Content_Node(ACTION_GENERATE_STATUSES_WINDOW, "All Messages", 12));
    list->addNode(new Content_Node(ACTION_TOGGLE_FLASHLIGHT, "Flashlight", 10));
    list->addNode(new Content_Node(ACTION_TOGGLE_SILENT_MODE, "Silent Mode", 11));
    list->addNode(new Content_Node(ACTION_SHUTDOWN_DEVICE, "Shutdown", 8));
    list->addNode(new Content_Node(ACTION_REBOOT_DEVICE, "Reboot Device", 13));

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
        currentWindow->currentState->processInput(inputID);
        currentWindow->switchWindowState(inputID);
    }
}

void OLED_Manager::callFunctionalWindowState(uint8_t inputID)
{
    if (currentWindow != nullptr)
    {
        currentWindow->currentState->processInput(inputID);
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
