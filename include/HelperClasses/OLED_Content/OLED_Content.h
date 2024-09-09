#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <map>
#include "globalDefines.h"
#include "System_Utils.h"
#include "Display_Utils.h"

// TODO: Get rid of this
enum class ContentType
{
    NONE,
    LIST,
    SETTINGS,
    COMPASS,
    GPS,
    LORA_TEST,
    STATUS,
    PING,
    TRACKING,
    EDIT_STRING,
    EDIT_INT,
    EDIT_FLOAT,
    EDIT_BOOL,
    EDIT_ENUM,
    HOME,
    SOS,
    SAVED_MSG,
    SAVE_CONFIRM,
    CONFIRM
};

// Struct for Callbacks
struct CallbackData
{
    uint32_t callbackID;
    char displayText[BUTTON_TEXT_MAX + 1];

    CallbackData()
    {
        callbackID = ACTION_NONE;
        displayText[0] = '\0';
    }

    CallbackData(const CallbackData &data)
    {
        callbackID = data.callbackID;
        strcpy(displayText, data.displayText);
    }

    // == operator
    bool operator==(const CallbackData &data)
    {
        return callbackID == data.callbackID && strcmp(displayText, data.displayText) == 0;
    }

    // != operator
    bool operator!=(const CallbackData &data)
    {
        return callbackID != data.callbackID || strcmp(displayText, data.displayText) != 0;
    }
};

// Abstract class for OLED content
class OLED_Content
{
public:
    //TODO: axe this
    ContentType type = ContentType::NONE;
    
    static Adafruit_SSD1306 *display;
    static QueueHandle_t displayCommandQueue;

    // map inputID to callback struct
    std::map<uint8_t, CallbackData> buttonCallbacks;

    virtual void encUp() {};
    virtual void encDown() {};
    virtual void printContent() = 0;

    virtual void start() {}
    virtual void stop() {}

    virtual void passButtonPress(uint8_t inputID) {}

    // TODO: Confine these to home screen
    static void drawBatteryIcon(size_t x, size_t y);
    static void drawBatteryIcon(size_t x, size_t y, uint8_t percentage);
    static void drawMessageIcon(size_t x, size_t y);
    static void drawBellIcon(size_t x, size_t y, bool isSilent);

    static void setTimerID(int timerID) { OLED_Content::refreshTimerID = timerID; }
    virtual ~OLED_Content() {}

protected:
    static int refreshTimerID;
};

class Content_Node
{
public:
    Content_Node(uint32_t resourceID, const char *nodeText, uint8_t textSize);
    ~Content_Node();

    uint32_t resourceID;
    Content_Node *next;
    Content_Node *prev;
    char nodeText[NODE_TEXT_MAX + 1];
    uint8_t textLength;
};

// Class for OLED content list implemented as circular doubly linked list
class OLED_Content_List : public OLED_Content
{
public:

    OLED_Content_List();
    OLED_Content_List(Adafruit_SSD1306 *display);
    ~OLED_Content_List();

    void addNode(Content_Node *node);
    void removeNode(Content_Node *node);
    Content_Node *getCurrentNode();

    void encUp();
    void encDown();

    void printContent();

private:
    Content_Node *head;
    Content_Node *current;
    uint8_t listSize;
};
