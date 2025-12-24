#pragma once

#include "OLED_Content.h"


class Edit_Float_Content : public OLED_Content
{
public:
    Edit_Float_Content()
    {
        type = ContentType::EDIT_FLOAT;
    }

    ~Edit_Float_Content()
    {
        ESP_LOGD(TAG, "Destructor");
    }

    void encDown()
    {
        this->floatToEdit += incrementAmt;
        if (this->floatToEdit > max)
        {
            this->floatToEdit = min;
        }
        // printContent();
    }

    void encUp()
    {
        this->floatToEdit -= incrementAmt;
        if (this->floatToEdit < min)
        {
            this->floatToEdit = max;
        }
        // printContent();
    }

    void printContent()
    {
        // Clear content area
        display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);
        display->setCursor(Display_Utils::centerTextHorizontal(6), Display_Utils::centerTextVertical());
        display->print(this->floatToEdit);
        ESP_LOGD(TAG, "printContent - value: %f, cursor: (%d, %d)",
                 this->floatToEdit, display->getCursorX(), display->getCursorY());
        display->display();
    }

    void editFloat(double floatToEdit, double min, double max, double incrementAmt = 0.1f)
    {
        ESP_LOGD(TAG, "editFloat");
        this->floatToEdit = floatToEdit;
        this->incrementAmt = incrementAmt;
        this->min = min;
        this->max = max;
    }

    double getFloat()
    {
        return this->floatToEdit;
    }

private:
    double floatToEdit = 0.0f;
    double incrementAmt = 0.1f;
    double min = 0.0f;
    double max = 0.0f;
};