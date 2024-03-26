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
        CallbackData backBtn;
        backBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(backBtn.displayText, "Back");
        this->buttonCallbacks[BUTTON_3] = backBtn;

        CallbackData confirmBtn;
        confirmBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(confirmBtn.displayText, "Confirm");
        this->buttonCallbacks[BUTTON_4] = confirmBtn;
    }

    ~Edit_Bool_State()
    {
        if (editBoolContent != nullptr)
        {
            editBoolContent->stop();
        }
    }

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            if (doc->containsKey("initialState"))
            {
                state = (*doc)["initialState"].as<bool>();
            }
        }

        displayState();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        DynamicJsonDocument *doc = new DynamicJsonDocument(64);
        (*doc)["return"] = state;

        transferData.serializedData = doc;
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
        OLED_Content::clearContentArea();

        if (state)
        {
            display->setCursor(OLED_Content::centerTextHorizontal(4), OLED_Content::selectTextLine(3));
            display->print("True");
        }
        else
        {
            display->setCursor(OLED_Content::centerTextHorizontal(5), OLED_Content::selectTextLine(3));
            display->print("False");
        }

        display->display();
    }

    bool state = false;
};

class Edit_Enum_State : public Window_State
{
public:
    Edit_Enum_State(Edit_Enum_Content *enumContent) : editEnumContent(enumContent), renderContent(enumContent)
    {
        CallbackData backBtn;
        backBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(backBtn.displayText, "Back");
        this->buttonCallbacks[BUTTON_3] = backBtn;

        CallbackData confirmBtn;
        confirmBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(confirmBtn.displayText, "Confirm");
        this->buttonCallbacks[BUTTON_4] = confirmBtn;
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
                if (doc->containsKey("valueArray") && doc->containsKey("textArray") && doc->containsKey("selectedIndex"))
                {
                    Enum_Data data;
                    data.valueArray = (*doc)["valueArray"].as<ArduinoJson::JsonArray>();
                    data.textArray = (*doc)["textArray"].as<ArduinoJson::JsonArray>();
                    data.selectedIndex = (*doc)["selectedIndex"].as<size_t>();

                    editEnumContent->assignEnum(data);
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

        if (editEnumContent != nullptr)
        {
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

private:
    Edit_Enum_Content *editEnumContent;

    // Keep a reference to the input arguments. This class will be responsible for deleting it
    DynamicJsonDocument *inputArgs;
};

class Edit_Float_State : public Window_State
{
public:
    Edit_Float_State(Edit_Float_Content *floatContent) : editFloatContent(floatContent), renderContent(floatContent)
    {
        CallbackData backBtn;
        backBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(backBtn.displayText, "Back");
        this->buttonCallbacks[BUTTON_3] = backBtn;

        CallbackData confirmBtn;
        confirmBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(confirmBtn.displayText, "Confirm");
        this->buttonCallbacks[BUTTON_4] = confirmBtn;
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
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            // Extract values if found and call editFloat
            if (doc->containsKey("value") && doc->containsKey("min") && doc->containsKey("max") && doc->containsKey("step"))
            {
                float value = (*doc)["value"].as<float>();
                float min = (*doc)["min"].as<float>();
                float max = (*doc)["max"].as<float>();
                float step = (*doc)["step"].as<float>();

                editFloatContent->editFloat(value, min, max, step);
            }
        }

        displayState();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        DynamicJsonDocument *doc = new DynamicJsonDocument(64);
        (*doc)["return"] = editFloatContent->getFloat();

        transferData.serializedData = doc;
    }

    void processInput(uint8_t inputID)
    {
        switch (inputID)
        {
        case ENC_UP:
            editFloatContent->encUp() break;
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
    Edit_Int_State(Edit_Int_Content *intContent) : editIntContent(intContent), renderContent(intContent)
    {
        CallbackData backBtn;
        backBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(backBtn.displayText, "Back");
        this->buttonCallbacks[BUTTON_3] = backBtn;

        CallbackData confirmBtn;
        confirmBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(confirmBtn.displayText, "Confirm");
        this->buttonCallbacks[BUTTON_4] = confirmBtn;
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
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            // Extract values if found and call editInt
            if (doc->containsKey("value") && doc->containsKey("min") && doc->containsKey("max") && doc->containsKey("step") && doc->containsKey("signed"))
            {
                isSigned = (*doc)["signed"].as<bool>();
                int value = (*doc)["value"].as<int>();
                int min = (*doc)["min"].as<int>();
                int max = (*doc)["max"].as<int>();
                int step = (*doc)["step"].as<int>();

                if (isSigned)
                {
                    editIntContent->editSignedInt(value, min, max, step);
                }
                else
                {
                    editIntContent->editUnsignedInt(value, min, max, step);
                }
            }
        }

        displayState();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

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
    Edit_String_State(Edit_String_Content *stringContent) : editStringContent(stringContent), renderContent(stringContent)
    {
        CallbackData backBtn;
        backBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(backBtn.displayText, "Back");
        this->buttonCallbacks[BUTTON_3] = backBtn;

        CallbackData confirmBtn;
        confirmBtn.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        strncpy(confirmBtn.displayText, "Confirm");
        this->buttonCallbacks[BUTTON_4] = confirmBtn;
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
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr)
        {
            ArduinoJson::DynamicJsonDocument *doc = transferData.serializedData;
            // Extract values if found and call editString
            if (doc->containsKey("str") && doc->containsKey("maxLen") && doc->containsKey("cursorPos"))
            {
                const char *str = (*doc)["str"].as<const char *>();
                int maxLength = (*doc)["maxLen"].as<size_t>();
                int cursorPos = (*doc)["cursorPos"].as<size_t>();

                editStringContent->setString(value, maxLength, cursorPos);
            }
        }

        displayState();
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);

        DynamicJsonDocument *doc = new DynamicJsonDocument(64);
        (*doc)["return"] = editStringContent->getString();

        transferData.serializedData = doc;
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
        }
    }

private:
    Edit_String_Content *editStringContent;
};
