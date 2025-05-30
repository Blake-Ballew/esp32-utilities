#include "Settings_Window.h"

Settings_Window::Settings_Window(OLED_Window *parent) : OLED_Window(parent)
{
    settingsState = new Settings_State();
    editBoolState = new Edit_Bool_State();
    editIntState = new Edit_Int_State();
    editStringState = new Edit_String_State();
    editFloatState = new Edit_Float_State();
    editEnumState = new Edit_Enum_State();

    stateList.push_back(settingsState);
    stateList.push_back(editBoolState);
    stateList.push_back(editIntState);
    stateList.push_back(editStringState);
    stateList.push_back(editFloatState);
    stateList.push_back(editEnumState);

    contentList.push_back(settingsState->renderContent);
    contentList.push_back(editBoolState->renderContent);
    contentList.push_back(editIntState->renderContent);
    contentList.push_back(editStringState->renderContent);
    contentList.push_back(editFloatState->renderContent);
    contentList.push_back(editEnumState->renderContent);

    if (FilesystemModule::Utilities::SettingsFile().isNull())
    {
        Settings_Manager::flashSettings();
    }

    this->saveSettings = false;

    setInitialState(settingsState);
}

Settings_Window::~Settings_Window()
{
}

void Settings_Window::callFunctionState(uint8_t inputID)
{
    if (currentState == settingsState)
    {
        bool initializeEditSuccess = false;
        JsonVariantType editType = JSON_VARIANT_TYPE_NULL;
        auto valueToEdit = settingsState->initializeEdit(initializeEditSuccess, editType);

        if (initializeEditSuccess)
        {
            State_Transfer_Data transferData;
            transferData.callbackID = ACTION_CALL_FUNCTIONAL_WINDOW_STATE;
            transferData.inputID = inputID;
            transferData.oldState = currentState;
            transferData.serializedData = valueToEdit;

            switch (editType)
            {
            case JSON_VARIANT_CONFIGURABLE_BOOL:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editBoolState");
#endif
                transferData.newState = editBoolState;
                break;
            case JSON_VARIANT_CONFIGURABLE_INTEGER:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editIntState");
#endif
                transferData.newState = editIntState;
                break;
            case JSON_VARIANT_CONFIGURABLE_STRING:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editStringState");
#endif
                transferData.newState = editStringState;
                break;
            case JSON_VARIANT_CONFIGURABLE_FLOAT:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editFloatState");
#endif
                transferData.newState = editFloatState;
                break;
            case JSON_VARIANT_CONFIGURABLE_ENUM:
#if DEBUG == 1
                Serial.println("Settings_Window::callFunctionState(): editEnumState");
#endif
                transferData.newState = editEnumState;
                break;
            default:
                break;
            }

            transferState(transferData);
        }
    }
}

void Settings_Window::returnFromFunctionState(uint8_t inputID)
{
    if (currentState == editBoolState ||
        currentState == editIntState ||
        currentState == editStringState ||
        currentState == editFloatState ||
        currentState == editEnumState)
    {
        State_Transfer_Data transferData;
        transferData.callbackID = ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE;
        transferData.inputID = inputID;
        transferData.oldState = currentState;
        transferData.newState = settingsState;
        transferData.serializedData = nullptr;

        transferState(transferData);
    }
}

void Settings_Window::transferState(State_Transfer_Data &transferData)
{
#if DEBUG == 1
    Serial.println("Settings_Window::transferState()");
#endif

    transferData.oldState->exitState(transferData);

    // Child classes will process transfer data coming out of the old state if needed

    transferData.newState->enterState(transferData);

    // processStateChangeReturnCode(transferData.returnCode);

    currentState = transferData.newState;

    // Clean up
    if (transferData.serializedData != nullptr)
    {
        delete transferData.serializedData;
    }
}

