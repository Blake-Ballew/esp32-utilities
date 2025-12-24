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
        ESP_LOGD(TAG, "Destructor");
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
        ESP_LOGD(TAG, "printContent");
        // Clear content area
        display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

        if (isSigned)
        {
            display->setCursor(Display_Utils::centerTextHorizontal(Display_Utils::getIntLength(signedInt)), Display_Utils::centerTextVertical());
            ESP_LOGV(TAG, "signedInt: %d, cursor: (%d, %d)",
                     signedInt, display->getCursorX(), display->getCursorY());
            display->print(signedInt);
        }
        else
        {
            display->setCursor(Display_Utils::centerTextHorizontal(Display_Utils::getUintLength(unsignedInt)), Display_Utils::centerTextVertical());
            ESP_LOGV(TAG, "unsignedInt: %u, cursor: (%d, %d)",
                     unsignedInt, display->getCursorX(), display->getCursorY());
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
        ESP_LOGD(TAG, "editUnsignedInt: intToEdit=%u, min=%u, max=%u, incrementAmt=%u",
                 intToEdit, min, max, incrementAmt);

        isSigned = false;
        unsignedInt = intToEdit;
        this->incrementAmt = incrementAmt;
        unsignedMin = min;
        unsignedMax = max;

        ESP_LOGD(TAG, "editUnsignedInt: end");
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