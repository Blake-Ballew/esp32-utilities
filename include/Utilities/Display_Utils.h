#pragma once

#include "Adafruit_GFX.h"
#include "System_Utils.h"

enum CommandType
{
    INPUT_COMMAND = 0,
    CALLBACK_COMMAND
};

struct DisplayCommandQueueItem
{
    CommandType commandType;

    union
    {
        struct
        {
            uint8_t inputID;
        } inputCommand;

        struct
        {
            uint32_t resourceID;
        } callbackCommand;
    } commandData;
};

class Display_Utils
{
public:

    // Setters
    static void setDisplay(Adafruit_GFX *display) { Display_Utils::display = display; }
    static void setDisplayDimensions(size_t width, size_t height)
    {
        displayWidth = width;
        displayHeight = height;
    }
    static void setRefreshTimerID(int timerID) { refreshTimerID = timerID; }
    static void setDisplayCommandQueue(QueueHandle_t queue) { displayCommandQueue = queue; }

    // Getters
    static Adafruit_GFX *getDisplay() { return display; }
    static size_t getDisplayWidth() { return displayWidth; }
    static size_t getDisplayHeight() { return displayHeight; }
    static QueueHandle_t getDisplayCommandQueue() { return displayCommandQueue; }

    // Graphics Helper Functions

    // Clears the content area. The content area is the area between the top and bottom text lines.
    static void clearContentArea() { display->fillRect(0, 8, displayWidth, displayHeight - 16, BLACK); }

    // Prints the given string in the center of the display.
    // If clearDisplay is true, the content area will be cleared first
    static void printCenteredText(const char *text, bool clearDisplay = true)
    {
        if (clearDisplay)
        {
            clearContentArea();
        }
        display->setCursor(centerTextHorizontal(text), centerTextVertical());
        display->print(text);
    }

    // Returns the Y cursor position for centering text vertically
    static uint16_t centerTextVertical() { return (displayHeight / 2) - 4; }

    // Returns the X cursor position for centering text horizontally for text of length textSize
    static uint16_t centerTextHorizontal(size_t textSize) { return (displayWidth / 2) - (textSize * 3); }

    // Returns the X cursor position for centering text horizontally for using the length of the string
    static uint16_t centerTextHorizontal(const char *text) { return centerTextHorizontal(strlen(text)); }

    // Returns the Y cursor position for selecting a text line
    static uint16_t selectTextLine(uint8_t line) { return (line - 1) * 8; }

    // Returns the X cursor position for aligning text to the left
    // distanceFrom is the spacing from the left edge of the display in characters
    static uint16_t alignTextLeft(size_t distanceFrom = 0) { return distanceFrom * 6; }

    // Returns the X cursor position for aligning text to the right using the length of the text
    static uint16_t alignTextRight(size_t textSize) { return displayWidth - (textSize * 6); }

    // Returns the X cursor position for aligning text to the right using the length of the string
    static uint16_t alignTextRight(const char *text) { return alignTextRight(strlen(text)); }

    // Returns the number of characters in an integer
    static size_t getIntLength(int64_t num)
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
    static size_t getUintLength(uint64_t num)
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


    // Command Queue Functions

    // Sends an input command to the display command queue
    static void sendInputCommand(uint8_t inputID)
    {
        if (displayCommandQueue == nullptr)
            return;

        DisplayCommandQueueItem item;

        item.commandType = INPUT_COMMAND;
        item.commandData.inputCommand.inputID = inputID;

        xQueueSend(displayCommandQueue, &item, portMAX_DELAY);
    }

    // Sends a callback command to the display command queue
    static void sendCallbackCommand(uint32_t resourceID)
    {
        if (displayCommandQueue == nullptr)
            return;

        DisplayCommandQueueItem item;

        item.commandType = CALLBACK_COMMAND;
        item.commandData.callbackCommand.resourceID = resourceID;

        xQueueSend(displayCommandQueue, &item, portMAX_DELAY);
    }

protected:
    static inline Adafruit_GFX *display = nullptr;

    static inline size_t displayWidth = 0;
    static inline size_t displayHeight = 0;

    static inline int refreshTimerID = -1;

    static inline QueueHandle_t displayCommandQueue = nullptr;
};