#pragma once

#include "Adafruit_GFX.h"
#include "System_Utils.h"
#include "EventHandler.h"

enum TextAlignmentHorizontal
{
    ALIGN_LEFT = 0,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

enum TextAlignmentVertical
{
    ALIGN_TOP = 0,
    ALIGN_CENTER,
    ALIGN_BOTTOM,
    CONTENT_TOP,
    CONTENT_BOTTOM,
    TEXT_LINE
};

struct TextFormat 
{
    TextAlignmentHorizontal horizontalAlignment;
    TextAlignmentVertical verticalAlignment;
    uint8_t line;
    int distanceFrom;
};

struct TextDrawData
{
    const char *text;
    TextFormat format;
};

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
    static void setDisplay(Adafruit_GFX *display);
    static void setDisplayDimensions(size_t width, size_t height);
    static void setRefreshTimerID(int timerID);
    static void setDisplayCommandQueue(QueueHandle_t queue);

    // Getters
    static Adafruit_GFX *getDisplay();
    static size_t getDisplayWidth();
    static size_t getDisplayHeight();
    static QueueHandle_t getDisplayCommandQueue();

    // Event Handlers
    static EventHandlerT<uint8_t> &getInputRaised() { return inputRaised; }

    // Graphics Helper Functions

    // Clears the content area. The content area is the area between the top and bottom text lines.
    static void clearContentArea();

    // Prints the given string in the center of the display.
    // If clearDisplay is true, the content area will be cleared first
    static void printCenteredText(const char *text, bool clearDisplay = true);

    // Returns the Y cursor position for centering text vertically
    static uint16_t centerTextVertical();

    // Returns the X cursor position for centering text horizontally for text of length textSize
    static uint16_t centerTextHorizontal(size_t textSize, int distanceFrom = 0);

    // Returns the X cursor position for centering text horizontally for using the length of the string
    static uint16_t centerTextHorizontal(const char *text, int distanceFrom = 0);

    // Returns the Y cursor position for selecting a text line
    static uint16_t selectTextLine(uint8_t line);

    // Prints a formatted string to the display
    static void printFormattedText(const char *text, TextFormat &format);

    // Returns the X cursor position for aligning text to the left
    // distanceFrom is the spacing from the left edge of the display in characters
    static uint16_t alignTextLeft(int distanceFrom = 0);

    // Returns the X cursor position for aligning text to the right using the length of the text
    static uint16_t alignTextRight(size_t textSize, int distanceFrom = 0);

    // Returns the X cursor position for aligning text to the right using the length of the string
    static uint16_t alignTextRight(const char *text, int distanceFrom = 0);

    // Returns the number of characters in an integer
    static size_t getIntLength(int64_t num);

    // Returns the number of characters in an unsigned integer
    static size_t getUintLength(uint64_t num);

    // Enables the refresh timer
    static void enableRefreshTimer(size_t timerPeriodMS = 0);

    // Disables the refresh timer
    static void disableRefreshTimer();


    // Command Queue Functions

    // Sends an input command to the display command queue
    static void sendInputCommand(uint8_t inputID);

    // Sends a callback command to the display command queue
    static void sendCallbackCommand(uint32_t resourceID);

protected:
    static Adafruit_GFX *display;

    static size_t displayWidth;
    static size_t displayHeight;

    static int refreshTimerID;

    static QueueHandle_t displayCommandQueue;

    // Event handlers
    static EventHandlerT<uint8_t> inputRaised;
};