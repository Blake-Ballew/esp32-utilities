#pragma once

#include "OLED_Content.h"

class Edit_String_Content : public OLED_Content
{
public:
    Edit_String_Content()
    {
        type = ContentType::EDIT_STRING;
        currStr = nullptr;
    }

    ~Edit_String_Content()
    {
#if DEBUG == 1
        Serial.println("Edit_String_Content destructor");
#endif
        stop();
        if (currStr != nullptr)
        {
            delete[] currStr;
        }
    }

    void printContent()
    {
#if DEBUG == 1
        Serial.println("Edit_String_Content::printContent");
#endif
        if (currStr == nullptr)
        {
            return;
        }

        // Clear content area
        display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

        // Print string
        display->setCursor(OLED_Content::centerTextHorizontal(cursorPos + 1), OLED_Content::selectTextLine(3));
        display->print(currStr);
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
        display->setCursor(OLED_Content::centerTextHorizontal(OLED_Content::getUintLength(currStrLen) + OLED_Content::getUintLength(cursorPos) + 3), OLED_Content::selectTextLine(2));
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

    void setString(const char *str, size_t maxLen, size_t pos)
    {
#if DEBUG == 1
        Serial.printf("Edit_String_Content::setString(%s, %d, %d)\n", str, maxLen, pos);
#endif
        if (currStr != nullptr)
        {
#if DEBUG == 1
            Serial.println("Edit_String_Content::setString - deleting old char[]");
#endif
            delete[] currStr;
        }
#if DEBUG == 1
        Serial.println("Edit_String_Content::setString - new char[]: ");
#endif
        currStr = new char[maxLen + 1];
#if DEBUG == 1
        Serial.println("Edit_String_Content::setString - copying memory ");
#endif
        memcpy(currStr, str, maxLen);
        currStr[maxLen] = '\0';
        currStrLen = maxLen;
        cursorPos = pos;
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
                currStr[cursorPos] = '\0';
            }
        }

        // Enter
        if (inputID == 4)
        {
            if (cursorPos < currStrLen)
            {
                currStr[cursorPos] = legalChars[cursorCharPos];
                cursorPos++;
                currStr[cursorPos] = '\0';
            }
        }
    }

    const char *getString() { return currStr; }

    void clearString()
    {
        delete[] currStr;
        currStr = nullptr;
    }

private:
    const static char legalChars[];

    size_t cursorCharPos = 0;
    size_t cursorPos = 0;
    char *currStr;
    size_t currStrLen;

    const size_t timerPeriodMS = 500;
    bool displayCursor = false;
};