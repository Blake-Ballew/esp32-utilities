#pragma once

#include "OLED_Content.h"

class Edit_String_Content : public OLED_Content
{
public:
    Edit_String_Content()
    {
        type = ContentType::EDIT_STRING;
    }

    ~Edit_String_Content()
    {
#if DEBUG == 1
        Serial.println("Edit_String_Content destructor");
#endif
        stop();
    }

    void printContent()
    {
#if DEBUG == 1
        Serial.println("Edit_String_Content::printContent");
#endif

        // Clear content area
        display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

        // Print string
        display->setCursor(Display_Utils::centerTextHorizontal(cursorPos + 1), Display_Utils::selectTextLine(3));
        display->print(currStr.c_str());
        if (displayCursor)
        {
            display->print('_');
        }
        else
        {
            display->print(legalChars[cursorCharPos]);
        }

        displayCursor = !displayCursor;

        // print max length and current length
        display->setCursor(Display_Utils::centerTextHorizontal(Display_Utils::getUintLength(currStrLen) + Display_Utils::getUintLength(cursorPos) + 3), Display_Utils::selectTextLine(2));
        display->printf("(%d/%d)", cursorPos, currStrLen);

        display->display();
    }

    void encUp()
    {
        if (cursorCharPos == 0)
        {
            cursorCharPos = strlen(legalChars) - 1;
        }
        else
        {
            cursorCharPos--;
        }

        displayCursor = false;
    }

    void encDown()
    {
        cursorCharPos++;
        if (cursorCharPos > strlen(legalChars) - 1)
        {
            cursorCharPos = 0;
        }

        displayCursor = false;
    }

    void start()
    {
        System_Utils::changeTimerPeriod(refreshTimerID, timerPeriodMS);
        System_Utils::startTimer(refreshTimerID);
    }

    void stop()
    {
        System_Utils::stopTimer(refreshTimerID);
    }

    void setString(std::string str, size_t maxLen)
    {
#if DEBUG == 1
        Serial.print("Editing string: ");
        Serial.println(str.c_str());
#endif

        currStr = str;
        currStrLen = maxLen;
        cursorPos = str.length();
        cursorCharPos = 0;
    }

    void setString(size_t maxLen)
    {
        currStr.clear();
        currStrLen = maxLen;
        cursorPos = 0;
        cursorCharPos = 0;
    }

    void passButtonPress(uint8_t inputID)
    {
        // Delete
        if (inputID == 1)
        {
            if (cursorPos > 0)
            {
                cursorPos--;
                currStr.erase(cursorPos, 1);
            }
        }

        // Enter
        if (inputID == 4)
        {
            if (cursorPos < currStrLen)
            {
                currStr.append(1, legalChars[cursorCharPos]);
                cursorPos++;
            }
        }
    }

    std::string getString() { return currStr; }

    void clearString()
    {
        currStr.clear();
        currStrLen = 0;
        cursorPos = 0;
        cursorCharPos = 0;
    }

private:
    const static char legalChars[];

    size_t cursorCharPos = 0;
    size_t cursorPos = 0;
    std::string currStr;
    size_t currStrLen;

    const size_t timerPeriodMS = 500;
    bool displayCursor = false;
};