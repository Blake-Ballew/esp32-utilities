#pragma once

#include "OLED_Content.h"


class Text_Display_Content : public OLED_Content
{
public:
    Text_Display_Content(std::vector<TextDrawData> txtData)
    {
        this->textData = txtData;
    }

    Text_Display_Content() {}

    void printContent()
    {
        ESP_LOGD(TAG, "printContent");
        for (auto txt : textData)
        {
            Display_Utils::printFormattedText(txt.text.c_str(), txt.format);
            ESP_LOGV(TAG, "printContent: %s", txt.text.c_str());
        }

        display->display();
    }

    void SetTextData(std::vector<TextDrawData> txtData) { this->textData = txtData; }
    
protected:

    std::vector<TextDrawData> textData;
};