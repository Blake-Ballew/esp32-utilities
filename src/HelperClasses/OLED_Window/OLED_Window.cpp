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
    display->setCursor(0, 0);
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

    if (currentState != nullptr)
    {
        currentState->displayState();
    }
}

OLED_Window *OLED_Window::getParentWindow()
{
    return parentWindow;
}

void OLED_Window::execBtnCallback(uint8_t buttonNumber, void *arg)
{
    CallbackData callback;

    if (currentState != nullptr)
    {
        if (currentState->buttonCallbacks.find(buttonNumber) != currentState->buttonCallbacks.end())
        {
            callback = currentState->buttonCallbacks[buttonNumber];
            if (callback.callbackID == ACTION_DEFER_CALLBACK_TO_CONTENT &&
                currentState->renderContent != nullptr)
            {
                content->passButtonPress(buttonNumber);
            }
        }
    }
    else
    {
        if (content != nullptr)
        {
            if (content->buttonCallbacks.find(buttonNumber) != content->buttonCallbacks.end())
            {
                callback = content->buttonCallbacks[buttonNumber];
            }
        }
    }
}

OLED_Window::~OLED_Window()
{
#if DEBUG == 1
    Serial.println("OLED_Window::~OLED_Window()");
#endif
    if (content != nullptr)
        delete content;
#if DEBUG == 1
    Serial.println("OLED_Window::~OLED_Window() Finished");
#endif
}
