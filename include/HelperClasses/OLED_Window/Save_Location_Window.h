#pragma once

#include "OLED_Window.h"
#include "globalDefines.h"
#include "Edit_States.h"
#include "Settings_Manager.h"
#include "Navigation_Manager.h"

class Save_Location_Window : public OLED_Window
{
public:
    Save_Location_Window(OLED_Window *parent) : OLED_Window(parent)
    {
        editStringState = new Edit_String_State();
        currentState = editStringState;

        stateList.push_back(currentState);

        State_Transfer_Data transferData;
        transferData.inputID = 0;
        transferData.callbackID = ACTION_NONE;
        transferData.oldState = nullptr;
        transferData.newState = currentState;

        DynamicJsonDocument doc(256);
        doc["cfgVal"] = "";
        doc["maxLen"] = OLED_WIDTH / 6;

        transferData.serializedData = &doc;

        editStringState->enterState(transferData);
    }

    ~Save_Location_Window() {}

    void returnFromFunctionState(uint8_t inputID)
    {
        if (currentState == nullptr)
        {
            return;
        }

        State_Transfer_Data transferData;

        transferData.inputID = inputID;
        transferData.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        transferData.serializedData = nullptr;
        transferData.oldState = currentState;
        transferData.newState = nullptr;

        editStringState->exitState(transferData);

        if (transferData.serializedData != nullptr && transferData.serializedData->containsKey("return")) 
        {
            char *newString = new char[strlen((*transferData.serializedData)["return"]) + 1];
            strcpy(newString, (*transferData.serializedData)["return"].as<const char *>());
            newString[strlen((*transferData.serializedData)["return"]) + 1] = '\0';

            auto coords = Navigation_Manager::getLocation();
            display->clearDisplay();

            if (!coords.isValid()) 
            {
                display->setCursor(OLED_Content::centerTextHorizontal("GPS Unavailable!"), OLED_Content::centerTextVertical());
                display->print("GPS Unavailable!");
                display->display();
                delete newString;
            }
            else
            {
                auto success = Settings_Manager::addCoordinate((const char *)newString, coords.lat(), coords.lng());
                

                if (success) 
                {
                    display->setCursor(OLED_Content::centerTextHorizontal("Location Saved!"), OLED_Content::centerTextVertical());
                    display->print("Location Saved!");
                }
                else
                {
                    display->setCursor(OLED_Content::centerTextHorizontal("Locations Full!"), OLED_Content::centerTextVertical());
                    display->print("Locations Full!");
                }
            }

            display->display();
            vTaskDelay(pdMS_TO_TICKS(2000));

            delete transferData.serializedData;

            // Queue ACTION_BACK
            DisplayCommandQueueItem queueBackCmd;
            queueBackCmd.commandType = CommandType::CALLBACK_COMMAND;
            queueBackCmd.commandData.callbackCommand.resourceID = ACTION_BACK;

            xQueueSend(displayCommandQueue, &queueBackCmd, pdMS_TO_TICKS(100));
        } 
        else if (transferData.serializedData != nullptr) 
        {
            delete transferData.serializedData;
        }
    }

protected:
        Edit_String_State *editStringState;
};