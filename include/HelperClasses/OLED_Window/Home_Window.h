#pragma once

#include "OLED_Window.h"
#include "Home_Content.h"
#include "Tracking_Content.h"
#include "Received_Messages_Content.h"
// #include "Saved_Locations_Content.h"
// #include "Saved_Messages_Content.h"
#include "LoraMessageDisplay.h"
#include "Text_Display_Content.h"

#include <string>

// #include "Received_Messages_State.h"
#include "UnreadMessageState.h"
#include "Home_State.h"
#include "Select_Message_State.h"
#include "Select_Location_State.h"
#include "SelectKeyValueState.h"
#include "SaveLocationState.h"
#include "Tracking_State.h"
#include "Lock_State.h"
#include "DisplaySentMessageState.h"
#include "Repeat_Message_State.h"

namespace
{
    const char *CURR_LOC PROGMEM = "Current Location";
}

class Home_Window : public OLED_Window
{
public:
    Home_Window(OLED_Window *parent);
    Home_Window();

    void encUp() {}
    void encDown() {}

    void drawWindow();

    void transferState(State_Transfer_Data &transferData) override;

private:
    Home_Content *homeContent;
    LoraMessageDisplay *messageDisplay;
    Text_Display_Content *textDisplay;

    Home_State *homeState;
    Tracking_State *trackingState;
    Select_Message_State *selectMessageState;
    Select_Location_State *selectLocationState;
    UnreadMessageState *unreadMessageState;
    SelectKeyValueState *selectionState;
    SaveLocationState *saveLocationState;
    Lock_State *lockState;
    DisplaySentMessageState *displaySentMessageState;
    Repeat_Message_State *repeatMessageState;

    // Message info
    bool sendDirect;
    uint64_t recipientID;
    bool useCurrLocation;
    int locationIdx;
    double latitude;
    double longitude;
    std::string locName;
    void clearMessageInfo();
};