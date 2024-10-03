#pragma once

#include "globalDefines.h"
#include "Window_State.h"
#include "ScrollWheel.h"
#include "LED_Utils.h"
#include <list>

struct MenuItem
{
    char text[(OLED_WIDTH / 6) + 1];
    CallbackData callback;
    Window_State *adjacentState;

    MenuItem(const char *text, uint32_t callbackID, Window_State *adjacentState)
    {
        auto strLength = strlen(text);
        if (strLength > OLED_WIDTH / 6)
        {
            strLength = OLED_WIDTH / 6;
        }
        strncpy(this->text, text, strLength);
        this->text[strLength] = '\0';
        this->callback.callbackID = callbackID;
        this->adjacentState = adjacentState;
    }

    MenuItem(const char *text, uint32_t callbackID)
    {
        MenuItem(text, callbackID, nullptr);
    }

    // Copy constructor
    MenuItem(const MenuItem &other)
    {
        strncpy(this->text, other.text, OLED_WIDTH / 6);
        this->text[OLED_WIDTH / 6] = '\0';
        this->callback = other.callback;
        this->adjacentState = other.adjacentState;
    }
};

class Menu_State : public Window_State
{
public:
    Menu_State()
    {
        currentMenuItem = menuItems.end();
        assignInput(ENC_UP, ACTION_DEFER_CALLBACK_TO_WINDOW, "Up");
        assignInput(ENC_DOWN, ACTION_DEFER_CALLBACK_TO_WINDOW, "Down");

        assignInput(BUTTON_3, ACTION_BACK, "Back");
        assignInput(BUTTON_4, ACTION_SELECT, "Select");
    }

    ~Menu_State()
    {
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);
        
        _ScrollWheelPatternID = ScrollWheel::RegisteredPatternID();
        LED_Utils::enablePattern(_ScrollWheelPatternID);
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);
        LED_Utils::disablePattern(_ScrollWheelPatternID);
    }

    void Pause()
    {
        LED_Utils::disablePattern(_ScrollWheelPatternID);
    }

    void Resume()
    {
        #if DEBUG == 1
        // Serial.println("Menu_State::Resume");
        #endif
        LED_Utils::enablePattern(_ScrollWheelPatternID);
        LED_Utils::iteratePattern(_ScrollWheelPatternID);
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
            if (currentMenuItem == menuItems.begin())
            {
                currentMenuItem = menuItems.end();
            }
            currentMenuItem--;
            break;
        case ENC_DOWN:
            currentMenuItem++;
            if (currentMenuItem == menuItems.end())
            {
                currentMenuItem = menuItems.begin();
            }
            break;
        default:
            break;
        }
    }

    void displayState()
    {
        #if DEBUG == 1
        // Serial.println("Menu_State::displayState");
        #endif
        
        Window_State::displayState();

        if (menuItems.size() == 0 || currentMenuItem == menuItems.end())
        {
            #if DEBUG == 1
            // Serial.println("No menu items");
            #endif
            return;
        }

        if (_ScrollWheelPatternID > -1)
        {
            StaticJsonDocument<64> doc;

            doc["numItems"] = menuItems.size();
            doc["currItem"] = std::distance(menuItems.begin(), currentMenuItem);

            #if DEBUG == 1
            // serializeJson(doc, Serial);
            #endif
            LED_Utils::configurePattern(_ScrollWheelPatternID, doc);
            LED_Utils::iteratePattern(_ScrollWheelPatternID);
        }

        #if DEBUG == 1
        // Serial.println("Rendering menu items");
        #endif

        const char *text = currentMenuItem->text;

        #if DEBUG == 1
        // Serial.print("Text: ");
        // Serial.println(text);
        // Serial.println("Setting cursor");
        #endif
        display->setCursor(Display_Utils::centerTextHorizontal(text), Display_Utils::centerTextVertical());
        #if DEBUG == 1
        // Serial.println("Printing text");
        #endif
        display->print(text);

        #if DEBUG == 1
        // Serial.println("Rendered menu item");
        #endif

        if (menuItems.size() > 1) 
        {
            display->setCursor(Display_Utils::centerTextHorizontal(1), Display_Utils::SelectTopTextLine());
            display->print("^");
            display->setCursor(Display_Utils::centerTextHorizontal(1), Display_Utils::selectTextLine(Display_Utils::SelectBottomTextLine()));
            display->print("v");
        }
    }

    void addMenuItem(const char *text, uint32_t callbackID, Window_State *adjacentState)
    {
        menuItems.push_back(MenuItem(text, callbackID, adjacentState));
        currentMenuItem = menuItems.begin();
    }

    void addMenuItem(const char *text, uint32_t callbackID)
    {
        addMenuItem(text, callbackID, nullptr);
        currentMenuItem = menuItems.begin();
    }

    CallbackData *getMenuItemCallback()
    {
        if (currentMenuItem == menuItems.end())
        {
            return nullptr;
        }

        return &currentMenuItem->callback;
    }

    Window_State *getAdjacentState()
    {
        if (currentMenuItem == menuItems.end())
        {
            return nullptr;
        }

        return currentMenuItem->adjacentState;
    }

protected:
    std::list<MenuItem> menuItems;
    std::list<MenuItem>::iterator currentMenuItem;

    int _ScrollWheelPatternID;
};