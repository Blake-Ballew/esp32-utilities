#pragma once

#include "OLED_Content.h"

class Edit_Int_Content : public OLED_Content
{
public:
    Edit_Int_Content()
    {
        type = ContentType::EDIT_INT;
    }

    ~Edit_Int_Content()
    {
#if DEBUG == 1
        Serial.println("Edit_Int_Content destructor");
#endif
    }

    void encDown()
    {
        if (isSigned)
        {
            signedInt += incrementAmt;
            if (signedInt > signedMax || signedInt < signedMin)
            {
                signedInt = signedMin;
            }
        }
        else
        {
            unsignedInt += incrementAmt;
            if (unsignedInt > unsignedMax || unsignedInt < unsignedMin)
            {
                unsignedInt = unsignedMin;
            }
        }
        // printContent();
    }

    void encUp()
    {
        if (isSigned)
        {
            signedInt -= incrementAmt;
            if (signedInt < signedMin || signedInt > signedMax)
            {
                signedInt = signedMax;
            }
        }
        else
        {
            unsignedInt -= incrementAmt;
            if (unsignedInt < unsignedMin || unsignedInt > unsignedMax)
            {
                unsignedInt = unsignedMax;
            }
        }
        // printContent();
    }

    void printContent()
    {
#if DEBUG == 1
        Serial.println("Edit_Int_Content::printContent()");
#endif
        // Clear content area
        display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

        if (isSigned)
        {
            display->setCursor(Display_Utils::centerTextHorizontal(Display_Utils::getIntLength(signedInt)), Display_Utils::centerTextVertical());
#if DEBUG == 1
            Serial.print("signedInt: ");
            Serial.println(signedInt);
            Serial.print("Cursor X: ");
            Serial.println(display->getCursorX());
            Serial.print("Cursor Y: ");
            Serial.println(display->getCursorY());
#endif
            display->print(signedInt);
        }
        else
        {
            display->setCursor(Display_Utils::centerTextHorizontal(Display_Utils::getUintLength(unsignedInt)), Display_Utils::centerTextVertical());
#if DEBUG == 1
            Serial.print("unsignedInt: ");
            Serial.println(unsignedInt);
            Serial.print("Cursor X: ");
            Serial.println(display->getCursorX());
            Serial.print("Cursor Y: ");
            Serial.println(display->getCursorY());
#endif
            display->print(unsignedInt);
        }
        display->display();
    }

    void editSignedInt(int32_t intToEdit, int32_t min, int32_t max, size_t incrementAmt = 1)
    {
        isSigned = true;
        signedInt = intToEdit;
        this->incrementAmt = incrementAmt;
        signedMin = min;
        signedMax = max;
    }

    void editUnsignedInt(uint32_t intToEdit, uint32_t min, uint32_t max, size_t incrementAmt = 1)
    {
        #if DEBUG == 1
        Serial.printf("Edit_Int_Content::editUnsignedInt: intToEdit: %u, min: %u, max: %u, incrementAmt: %u\n", intToEdit, min, max, incrementAmt);
        #endif
        

        isSigned = false;
        #if DEBUG == 1
        Serial.println("Edit_Int_Content::editUnsignedInt: after isSigned");
        #endif
        unsignedInt = intToEdit;
        #if DEBUG == 1
        Serial.println("Edit_Int_Content::editUnsignedInt: after unsignedInt");
        #endif
        this->incrementAmt = incrementAmt;
        #if DEBUG == 1
        Serial.println("Edit_Int_Content::editUnsignedInt: after incrementAmt");
        #endif
        unsignedMin = min;
        #if DEBUG == 1
        Serial.println("Edit_Int_Content::editUnsignedInt: after unsignedMin");
        #endif
        unsignedMax = max;
        #if DEBUG == 1
        Serial.println("Edit_Int_Content::editUnsignedInt: end");
        #endif
    }

    int32_t getSignedInt()
    {
        return signedInt;
    }

    uint32_t getUnsignedInt()
    {
        return unsignedInt;
    }

    bool getIsSigned()
    {
        return isSigned;
    }

private:
    uint32_t unsignedInt;
    int32_t signedInt;
    uint32_t unsignedMin;
    uint32_t unsignedMax;
    int32_t signedMin;
    int32_t signedMax;
    bool isSigned;
    size_t incrementAmt = 1;
};