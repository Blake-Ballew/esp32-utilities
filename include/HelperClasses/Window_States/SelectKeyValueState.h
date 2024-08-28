#pragma once

#include "Window_State.h"
#include <vector>
#include <string>

class SelectKeyValueState : public Window_State
{
public:
    SelectKeyValueState() 
    {
        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Select");
        assignInput(ENC_UP, ACTION_DEFER_CALLBACK_TO_WINDOW);
        assignInput(ENC_DOWN, ACTION_DEFER_CALLBACK_TO_WINDOW);
    }

    ~SelectKeyValueState() {}

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            DynamicJsonDocument *doc = transferData.serializedData;
            if (doc->containsKey("items"))
            {
                auto items = (*transferData.serializedData)["items"].as<JsonArray>();
                itemsToDisplay.clear();

                for (auto item : items)
                {
                    itemsToDisplay.push_back(std::pair<std::string, int>(item["key"].as<std::string>(), item["value"].as<int>()));
                }

                selected = itemsToDisplay.begin();
            }

            if (doc->containsKey("prompt"))
            {
                prompt = (*transferData.serializedData)["prompt"].as<std::string>();
            }
        }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        if (transferData.inputID == BUTTON_4)
        {
            transferData.serializedData = new DynamicJsonDocument(64);

            (*transferData.serializedData)["return"] = selected->second;
        }
    }

    void processInput(uint8_t inputID)
    {
        if (inputID == ENC_UP)
        {
            if (selected == itemsToDisplay.begin())
            {
                selected = itemsToDisplay.end();
            }

            selected--;
        }

        if (inputID == ENC_DOWN)
        {
            selected++;

            if (selected == itemsToDisplay.end())
            {
                selected = itemsToDisplay.begin();
            }
        }
    }

    void displayState()
    {
        Window_State::displayState();

        if (selected != itemsToDisplay.end())
        {
            TextFormat promptFormat;

            promptFormat.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            promptFormat.verticalAlignment = TEXT_LINE;
            promptFormat.line = 2;

            Display_Utils::printFormattedText(prompt.c_str(), promptFormat);

            TextFormat keyFormat;

            keyFormat.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            keyFormat.verticalAlignment = TEXT_LINE;
            keyFormat.line = 3;

            Display_Utils::printFormattedText(selected->first.c_str(), keyFormat);
        }
    }

protected:
    // Map of items to display. The key is displayed on the screen and the value is returned if selected
    std::vector<std::pair<std::string, int>> itemsToDisplay;
    std::vector<std::pair<std::string, int>>::iterator selected;

    std::string prompt;
};