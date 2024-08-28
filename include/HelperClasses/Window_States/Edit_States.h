#pragma once

#include "Window_State.h"
#include "Edit_Bool_Content.h"
#include "Edit_Enum_Content.h"
#include "Edit_Float_Content.h"
#include "Edit_Int_Content.h"
#include "Edit_String_Content.h"

class Edit_Bool_State : public Window_State
{
public:
    Edit_Bool_State()
    {
        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Confirm");
    }

    ~Edit_Bool_State()
    {
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            if (doc->containsKey("cfgVal"))
            {
                state = (*doc)["cfgVal"].as<bool>();
            }
        }

        displayState();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        if (transferData.inputID == BUTTON_4)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["return"] = state;

            transferData.serializedData = doc;
        }
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
            state = !state;
            break;
        case ENC_DOWN:
            state = !state;
            break;
        }
    }

    void displayState()
    {
        Display_Utils::clearContentArea();

        if (state)
        {
            display->setCursor(Display_Utils::centerTextHorizontal(4), Display_Utils::selectTextLine(3));
            display->print("True");
        }
        else
        {
            display->setCursor(Display_Utils::centerTextHorizontal(5), Display_Utils::selectTextLine(3));
            display->print("False");
        }

        display->display();
    }

    bool state = false;
};

class Edit_Enum_State : public Window_State
{
public:
    Edit_Enum_State()
    {
        editEnumContent = new Edit_Enum_Content();
        renderContent = editEnumContent;

        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Confirm");
    }

    Edit_Enum_State(Edit_Enum_Content *enumContent)
    {
        editEnumContent = enumContent;
        renderContent = enumContent;

        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Confirm");
    }

    ~Edit_Enum_State()
    {
        if (editEnumContent != nullptr)
        {
            editEnumContent->stop();
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            if (editEnumContent != nullptr)
            {
                ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
                if (doc->containsKey("vals") && doc->containsKey("valTxt") && doc->containsKey("cfgVal"))
                {
                    Enum_Data data;
                    data.valueArray = (*doc)["vals"].as<ArduinoJson::JsonArray>();
                    data.textArray = (*doc)["valTxt"].as<ArduinoJson::JsonArray>();
                    data.selectedIndex = (*doc)["cfgVal"].as<size_t>();

                    editEnumContent->assignEnum(&data);
                }

                // Keep a reference to the input arguments and nullify the pointer in the transfer data
                inputArgs = transferData.serializedData;
                transferData.serializedData = nullptr;
            }
        }

        Window_State::displayState();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        if (transferData.inputID == BUTTON_4)
        {
            // Get the selected index (value
            size_t returnIdx = editEnumContent->getSelectedIndex();

            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["return"] = returnIdx;
            transferData.serializedData = doc;

            if (inputArgs != nullptr)
            {
                delete inputArgs;
            }
        }
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
            editEnumContent->encUp();
            break;
        case ENC_DOWN:
            editEnumContent->encDown();
            break;
        }
    }

private:
    Edit_Enum_Content *editEnumContent;

    // Keep a reference to the input arguments. This class will be responsible for deleting it
    DynamicJsonDocument *inputArgs;
};

class Edit_Float_State : public Window_State
{
public:
    Edit_Float_State()
    {
        editFloatContent = new Edit_Float_Content();
        renderContent = editFloatContent;

        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Confirm");
    }

    Edit_Float_State(Edit_Float_Content *floatContent)
    {
        editFloatContent = floatContent;
        renderContent = floatContent;

        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Confirm");
    }

    ~Edit_Float_State()
    {
        if (editFloatContent != nullptr)
        {
            editFloatContent->stop();
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        #if DEBUG == 1
        Serial.println("Edit_Float_State::enterState");
        #endif
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            #if DEBUG == 1
            Serial.println("Edit_Float_State::enterState: serializedData not null");
            #endif
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            // Extract values if found and call editFloat
            if (doc->containsKey("cfgVal") && doc->containsKey("minVal") && doc->containsKey("maxVal") && doc->containsKey("incVal"))
            {
                #if DEBUG == 1
                Serial.printf("Edit_Float_State::enterState: cfgVal: %f, minVal: %f, maxVal: %f, incVal: %f\n", (*doc)["cfgVal"].as<float>(), (*doc)["minVal"].as<float>(), (*doc)["maxVal"].as<float>(), (*doc)["incVal"].as<float>());
                #endif
                float value = (*doc)["cfgVal"].as<float>();
                float min = (*doc)["minVal"].as<float>();
                float max = (*doc)["maxVal"].as<float>();
                float step = (*doc)["incVal"].as<float>();

                editFloatContent->editFloat(value, min, max, step);
            }
        }

        displayState();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        if (transferData.inputID == BUTTON_4)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["return"] = editFloatContent->getFloat();

            transferData.serializedData = doc;
        }
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
            editFloatContent->encUp();
            break;
        case ENC_DOWN:
            editFloatContent->encDown();
            break;
        }
    }

private:
    Edit_Float_Content *editFloatContent;
};

class Edit_Int_State : public Window_State
{
public:
    Edit_Int_State()
    {
        editIntContent = new Edit_Int_Content();
        renderContent = editIntContent;

        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Confirm");
    }

    Edit_Int_State(Edit_Int_Content *intContent)
    {
        editIntContent = intContent;
        renderContent = intContent;

        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Confirm");
    }

    ~Edit_Int_State()
    {
        if (editIntContent != nullptr)
        {
            editIntContent->stop();
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        #if DEBUG == 1
        Serial.println("Edit_Int_State::enterState");
        #endif
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            #if DEBUG == 1
            Serial.println("Edit_Int_State::enterState: serializedData not null");
            #endif
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            // Extract values if found and call editInt
            if (doc->containsKey("cfgVal") && doc->containsKey("minVal") && doc->containsKey("maxVal") && doc->containsKey("incVal") && doc->containsKey("signed"))
            {
                #if DEBUG == 1
                Serial.println("Edit_Int_State::enterState: cfgVal, minVal, maxVal, incVal, signed found");
                #endif
                isSigned = (*doc)["signed"].as<bool>();
                int value = (*doc)["cfgVal"].as<int>();
                int min = (*doc)["minVal"].as<int>();
                int max = (*doc)["maxVal"].as<int>();
                int step = (*doc)["incVal"].as<int>();

                if (isSigned)
                {
                    #if DEBUG == 1
                    Serial.println("Edit_Int_State::enterState: isSigned");
                    #endif
                    editIntContent->editSignedInt(value, min, max, step);
                }
                else
                {
                    #if DEBUG == 1
                    Serial.println("Edit_Int_State::enterState: isUnsigned");
                    #endif
                    editIntContent->editUnsignedInt(value, min, max, step);
                }
            }
        }

        displayState();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        if (transferData.inputID == BUTTON_4)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            if (isSigned)
            {
                (*doc)["return"] = editIntContent->getSignedInt();
            }
            else
            {
                (*doc)["return"] = editIntContent->getUnsignedInt();
            }

            transferData.serializedData = doc;
        }
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
            editIntContent->encUp();
            break;
        case ENC_DOWN:
            editIntContent->encDown();
            break;
        }
    }

private:
    Edit_Int_Content *editIntContent;
    bool isSigned;
};

class Edit_String_State : public Window_State
{
public:
    Edit_String_State()
    {
        editStringContent = new Edit_String_Content();
        renderContent = editStringContent;

        assignInput(BUTTON_1, ACTION_DEFER_CALLBACK_TO_WINDOW, "Delete");
        assignInput(BUTTON_2, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Confirm");
        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_DEFER_CALLBACK_TO_WINDOW, "Select");
    }

    Edit_String_State(Edit_String_Content *stringContent)
    {
        editStringContent = stringContent;
        renderContent = stringContent;

        assignInput(BUTTON_1, ACTION_DEFER_CALLBACK_TO_WINDOW, "Delete");
        assignInput(BUTTON_2, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Confirm");
        assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        assignInput(BUTTON_4, ACTION_DEFER_CALLBACK_TO_WINDOW, "Select");
    }

    ~Edit_String_State()
    {
        if (editStringContent != nullptr)
        {
            editStringContent->stop();
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
#if DEBUG == 1
        Serial.println("Edit_String_State::enterState");
#endif
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            // Extract values if found and call editString
            if (doc->containsKey("cfgVal") && doc->containsKey("maxLen"))
            {
#if DEBUG == 1
                Serial.print("Edit_String_State::enterState: cfgVal -  ");
                Serial.println((*doc)["cfgVal"].as<std::string>().c_str());
                Serial.print("Edit_String_State::enterState: maxLen -  ");
                Serial.println((*doc)["maxLen"].as<size_t>());
#endif
                auto str = (*doc)["cfgVal"].as<std::string>();
                size_t maxLength = (*doc)["maxLen"].as<size_t>();

                editStringContent->setString(str, maxLength);
            }
        }

        displayState();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        if (transferData.inputID == BUTTON_2)
        {
            DynamicJsonDocument *doc = new DynamicJsonDocument(64);
            (*doc)["return"] = editStringContent->getString();

            transferData.serializedData = doc;
        }

        editStringContent->clearString();
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
            editStringContent->encUp();
            break;
        case ENC_DOWN:
            editStringContent->encDown();
            break;
        case BUTTON_1:
            editStringContent->passButtonPress(1);
            break;
        case BUTTON_4:
            editStringContent->passButtonPress(4);
            break;
        }
    }

private:
    Edit_String_Content *editStringContent;
};
