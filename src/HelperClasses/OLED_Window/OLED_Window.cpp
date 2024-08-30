#include "OLED_Window.h"

Adafruit_SSD1306 *OLED_Window::display;

OLED_Window::OLED_Window()
{
    btnCallback = NULL;
    parentWindow = this;
    memset(btn1Text, '\0', BUTTON_TEXT_MAX);
    memset(btn2Text, '\0', BUTTON_TEXT_MAX);
    memset(btn3Text, '\0', BUTTON_TEXT_MAX);
    memset(btn4Text, '\0', BUTTON_TEXT_MAX);
    content = NULL;
}

OLED_Window::OLED_Window(OLED_Window *parent)
{
    btnCallback = NULL;
    parentWindow = parent;
    memset(btn1Text, '\0', BUTTON_TEXT_MAX);
    memset(btn2Text, '\0', BUTTON_TEXT_MAX);
    memset(btn3Text, '\0', BUTTON_TEXT_MAX);
    memset(btn4Text, '\0', BUTTON_TEXT_MAX);
    content = NULL;
}

// TODO: Get rid of this after refactoring all windows
void OLED_Window::assignButton(uint32_t callbackID, int buttonNumber, const char *buttonText, uint8_t textLength)
{
    if (textLength > BUTTON_TEXT_MAX)
        textLength = BUTTON_TEXT_MAX;
    switch (buttonNumber)
    {
    case BUTTON_1:
        btn1CallbackID = callbackID;
        if (buttonText == NULL || buttonText[0] == '\0')
        {
            memset(btn1Text, '\0', BUTTON_TEXT_MAX);
            break;
        }
        strncpy(btn1Text, buttonText, textLength);
        btn1Text[textLength] = '\0'; // Make sure string is null terminated
        break;
    case BUTTON_2:
        btn2CallbackID = callbackID;
        if (buttonText == NULL || buttonText[0] == '\0')
        {
            memset(btn2Text, '\0', BUTTON_TEXT_MAX);
            break;
        }
        strncpy(btn2Text, buttonText, textLength);
        btn2Text[textLength] = '\0'; // Make sure string is null terminated
        break;
    case BUTTON_3:
        btn3CallbackID = callbackID;
        if (buttonText == NULL || buttonText[0] == '\0')
        {
            memset(btn3Text, '\0', BUTTON_TEXT_MAX);
            break;
        }
        strncpy(btn3Text, buttonText, textLength);
        btn3Text[textLength] = '\0'; // Make sure string is null terminated
        break;
    case BUTTON_4:
        btn4CallbackID = callbackID;
        if (buttonText == NULL || buttonText[0] == '\0')
        {
            memset(btn4Text, '\0', BUTTON_TEXT_MAX);
            break;
        }
        strncpy(btn4Text, buttonText, textLength);
        btn4Text[textLength] = '\0'; // Make sure string is null terminated
        break;
    default:
        break;
    }
}

void OLED_Window::drawWindow()
{
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display->fillRect(0, 0, OLED_WIDTH, 8, SSD1306_BLACK);
    display->fillRect(0, OLED_HEIGHT - 8, OLED_WIDTH, 8, SSD1306_BLACK);

    if (currentState != nullptr)
    {
#if DEBUG == 1
        // Serial.println("State is not null");
#endif
        if (currentState->buttonCallbacks.find(BUTTON_1) != currentState->buttonCallbacks.end())
        {
            display->setCursor(0, 0);

            display->print(currentState->buttonCallbacks[BUTTON_1].displayText);
        }

        if (currentState->buttonCallbacks.find(BUTTON_2) != currentState->buttonCallbacks.end())
        {
            display->setCursor(OLED_WIDTH - (strlen(currentState->buttonCallbacks[BUTTON_2].displayText) * 6), 0);
            display->print(currentState->buttonCallbacks[BUTTON_2].displayText);
        }

        if (currentState->buttonCallbacks.find(BUTTON_3) != currentState->buttonCallbacks.end())
        {
            display->setCursor(0, OLED_HEIGHT - 8);
            display->print(currentState->buttonCallbacks[BUTTON_3].displayText);
        }

        if (currentState->buttonCallbacks.find(BUTTON_4) != currentState->buttonCallbacks.end())
        {
            display->setCursor(OLED_WIDTH - (strlen(currentState->buttonCallbacks[BUTTON_4].displayText) * 6), OLED_HEIGHT - 8);
            display->print(currentState->buttonCallbacks[BUTTON_4].displayText);
        }

        #if DEBUG == 1
        // Serial.println("Rendered intputs");
        #endif
    }
#if DEBUG == 1
    else
    {
        // Serial.println("State is null");
    }
#endif

    /*
    if (btn1Text[0] != '\0')
        display->print(btn1Text);
    if (btn2Text[0] != '\0')
    {
        display->setCursor(OLED_WIDTH - (strlen(btn2Text) * 6), 0);
        display->print(btn2Text);
    }
    if (btn3Text[0] != '\0')
    {
        display->setCursor(0, OLED_HEIGHT - 8);
        display->print(btn3Text);
    }
    if (btn4Text[0] != '\0')
    {
        display->setCursor(OLED_WIDTH - (strlen(btn4Text) * 6), OLED_HEIGHT - 8);
        display->print(btn4Text);
    }
    if (content != NULL)
        content->printContent();
    else
        display->display();
    */
    #if DEBUG == 1
    // Serial.println("OLED_Window::drawWindow() before displayState()");
    #endif
    if (currentState != nullptr)
    {
        #if DEBUG == 1
        // Serial.println("OLED_Window::drawWindow() currentState is not null");
        #endif
        currentState->displayState();
    }
    else
    {
        #if DEBUG == 1
        // Serial.println("OLED_Window::drawWindow() currentState is null");
        #endif
        display->display();
    }
#if DEBUG == 1
    // Serial.println("OLED_Window::drawWindow() finished");
#endif

    display->display();
}

// TODO: Implement this in display manager
OLED_Window *OLED_Window::getParentWindow()
{
    return parentWindow;
}

void OLED_Window::execBtnCallback(uint8_t inputID)
{
#if DEBUG == 1
    // Serial.println("OLED_Window::execBtnCallback()");
#endif

    if (currentState != nullptr)
    {
        currentState->processInput(inputID);
    }
}

OLED_Window::~OLED_Window()
{
    if (currentState != nullptr)
    {
        currentState->exitState();
    }

    for (auto &it : stateList)
    {
        delete it;
    }

    for (auto &it : contentList)
    {
        delete it;
    }

    LED_Manager::clearRing();
}
