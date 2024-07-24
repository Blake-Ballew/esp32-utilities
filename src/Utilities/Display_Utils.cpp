#include "Display_Utils.h"

Adafruit_GFX *Display_Utils::display = nullptr;

size_t Display_Utils::displayWidth = 0;
size_t Display_Utils::displayHeight = 0;

int Display_Utils::refreshTimerID = -1;

QueueHandle_t Display_Utils::displayCommandQueue = nullptr;

EventHandlerT<uint8_t> Display_Utils::inputRaised;

// Setters
void Display_Utils::setDisplay(Adafruit_GFX *display) 
{ 
    Display_Utils::display = display; 
}

void Display_Utils::setDisplayDimensions(size_t width, size_t height)
{
    displayWidth = width;
    displayHeight = height;
}

void Display_Utils::setRefreshTimerID(int timerID) 
{
    refreshTimerID = timerID; 
}

void Display_Utils::setDisplayCommandQueue(QueueHandle_t queue) 
{ 
    displayCommandQueue = queue; 
}

// Getters
Adafruit_GFX *Display_Utils::getDisplay() { return display; }

size_t Display_Utils::getDisplayWidth() { return displayWidth; }

size_t Display_Utils::getDisplayHeight() { return displayHeight; }

QueueHandle_t Display_Utils::getDisplayCommandQueue() { return displayCommandQueue; }

// Graphics Helper Functions

// Clears the content area. The content area is the area between the top and bottom text lines.
void Display_Utils::clearContentArea() { display->fillRect(0, 8, displayWidth, displayHeight - 16, BLACK); }

// Prints the given string in the center of the display.
// If clearDisplay is true, the content area will be cleared first
void Display_Utils::printCenteredText(const char *text, bool clearDisplay)
{
    if (clearDisplay)
    {
        clearContentArea();
    }
    display->setCursor(centerTextHorizontal(text), centerTextVertical());
    display->print(text);
}

// Returns the Y cursor position for centering text vertically
uint16_t Display_Utils::centerTextVertical() { return (displayHeight / 2) - 4; }

// Returns the X cursor position for centering text horizontally for text of length textSize
uint16_t Display_Utils::centerTextHorizontal(size_t textSize, int distanceFrom) { return ((displayWidth / 2) - (textSize * 3)) + (distanceFrom * 6); }

// Returns the X cursor position for centering text horizontally for using the length of the string
uint16_t Display_Utils::centerTextHorizontal(const char *text, int distanceFrom) { return centerTextHorizontal(strlen(text), distanceFrom); }

// Returns the Y cursor position for selecting a text line
uint16_t Display_Utils::selectTextLine(uint8_t line) { return (line - 1) * 8; }

// Prints a formatted string to the display
void Display_Utils::printFormattedText(const char *text, TextFormat &format) 
{
    uint16_t xPos, yPos;

    switch (format.horizontalAlignment)
    {
    case ALIGN_LEFT:
        xPos = alignTextLeft(format.distanceFrom);
        break;
    case ALIGN_RIGHT:
        xPos = alignTextRight(text, format.distanceFrom);
        break;
    case ALIGN_CENTER:
        xPos = centerTextHorizontal(text);
        xPos += format.distanceFrom * 6;
        break;
    default:
        xPos = 0;
        break;
    }

    switch (format.verticalAlignment)
    {
    case ALIGN_TOP:
        yPos = 0;
        break;
    case ALIGN_BOTTOM:
        yPos = displayHeight - 8;
        break;
    case ALIGN_CENTER:
        yPos = centerTextVertical();
        break;
    case CONTENT_TOP:
        yPos = 8;
        break;
    case CONTENT_BOTTOM:
        yPos = displayHeight - 16;
        break;
    case TEXT_LINE:
        yPos = selectTextLine(format.line);
        break;
    default:
        yPos = 0;
        break;
    }

    display->setCursor(xPos, yPos);
    display->print(text);
}

// Returns the X cursor position for aligning text to the left
// distanceFrom is the spacing from the left edge of the display in characters
uint16_t Display_Utils::alignTextLeft(int distanceFrom) { return distanceFrom * 6; }

// Returns the X cursor position for aligning text to the right using the length of the text
uint16_t Display_Utils::alignTextRight(size_t textSize, int distanceFrom) { return (displayWidth - (textSize * 6) - (distanceFrom * 6)); }

// Returns the X cursor position for aligning text to the right using the length of the string
uint16_t Display_Utils::alignTextRight(const char *text, int distanceFrom) { return alignTextRight(strlen(text), distanceFrom); }

// Returns the number of characters in an integer
size_t Display_Utils::getIntLength(int64_t num)
{
    size_t len = 0;
    if (num == 0)
    {
        return 1;
    }
    if (num < 0)
    {
        len++;
        num *= -1;
    }
    while (num > 0)
    {
        len++;
        num /= 10;
    }
    return len;
}

// Returns the number of characters in an unsigned integer
size_t Display_Utils::getUintLength(uint64_t num)
{
    size_t len = 0;
    if (num == 0)
    {
        return 1;
    }
    while (num > 0)
    {
        len++;
        num /= 10;
    }
    return len;
}

void Display_Utils::enableRefreshTimer(size_t timerPeriodMS)
{
    if (refreshTimerID == -1)
        return;

    if (timerPeriodMS > 0)
        System_Utils::changeTimerPeriod(refreshTimerID, timerPeriodMS);

    System_Utils::startTimer(refreshTimerID);
}

void Display_Utils::disableRefreshTimer()
{
    if (refreshTimerID == -1)
        return;

    System_Utils::stopTimer(refreshTimerID);
}


// Command Queue Functions

// Sends an input command to the display command queue
void Display_Utils::sendInputCommand(uint8_t inputID)
{
    if (displayCommandQueue == nullptr)
        return;

    DisplayCommandQueueItem item;

    item.commandType = INPUT_COMMAND;
    item.commandData.inputCommand.inputID = inputID;

    xQueueSend(displayCommandQueue, &item, portMAX_DELAY);
}

// Sends a callback command to the display command queue
void Display_Utils::sendCallbackCommand(uint32_t resourceID)
{
    if (displayCommandQueue == nullptr)
        return;

    DisplayCommandQueueItem item;

    item.commandType = CALLBACK_COMMAND;
    item.commandData.callbackCommand.resourceID = resourceID;

    xQueueSend(displayCommandQueue, &item, portMAX_DELAY);
}

