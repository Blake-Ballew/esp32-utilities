#pragma once

#include "Window.hpp"
#include "States/SettingsState.hpp"
#include "States/EditBoolState.hpp"
#include "States/EditIntState.hpp"
#include "States/EditFloatState.hpp"
#include "States/EditEnumState.hpp"
#include "States/EditStringState.hpp"
#include "DisplayUtilities.hpp"
#include "FilesystemUtils.h"

namespace DisplayModule
{
    class SettingsWindow : public Window
    {
    public:
        SettingsWindow()
        {
            // Allocate states
            _settingsState = std::make_shared<SettingsState>();
            _editBool      = std::make_shared<EditBoolState>();
            _editInt       = std::make_shared<EditIntState>();
            _editFloat     = std::make_shared<EditFloatState>();
            _editEnum      = std::make_shared<EditEnumState>();
            _editString    = std::make_shared<EditStringState>();

            // Set initial state
            setInitialState(_settingsState);

            // BUTTON_4 — select (label is updated dynamically by SettingsState)
            _settingsState->bindInput(InputID::BUTTON_4, "Edit", [this](const InputContext &ctx)
                {
                    const auto selType = _settingsState->getSelectionType();

                    ESP_LOGI(TAG, "BUTTON_4 pressed in SettingsWindow; selection type = %s", selType);

                    // Leaf node: state builds the payload, window does the push
                    StateTransferData d = _settingsState->buildExitData(ctx.inputID);

                    if (selType == "bool")
                    {
                        pushState(_editBool, d);
                    }
                    else if (selType == "int")
                    {
                        pushState(_editInt, d);
                    }
                    else if (selType == "float")
                    {
                        pushState(_editFloat, d);
                    }
                    else if (selType == "enum")
                    {
                        pushState(_editEnum, d);
                    }
                    else if (selType == "string")
                    {
                        pushState(_editString, d);
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Unknown selection type '%s'; no edit state pushed", selType.c_str());
                    }

                    ESP_LOGI(TAG, "Pushed edit state for selection with payload: %s",
                             d.payload ? d.payload->as<std::string>().c_str() : "null");
                });

            // BUTTON_3 — back / exit / cancel edit
            _settingsState->bindInput(InputID::BUTTON_3, "Back", [](const InputContext &_) 
            {
                FilesystemModule::Utilities::RequestSettingsRefresh().Invoke();
                Utilities::popWindow();
            });

            // Each edit state fires its own confirm action — the window hands
            // it a lambda that pops the state with the result payload.
            auto confirmFn = [this](StateTransferData d){ popState(d); };
            auto cancellFn = [this](const InputContext &){ popState(); };

            _editBool  ->setConfirmAction(confirmFn);
            _editInt   ->setConfirmAction(confirmFn);
            _editFloat ->setConfirmAction(confirmFn);
            _editEnum  ->setConfirmAction(confirmFn);
            _editString->setConfirmAction(confirmFn);

            _editBool  ->bindInput(InputID::BUTTON_3, "Cancel", cancellFn);
            _editInt   ->bindInput(InputID::BUTTON_3, "Cancel", cancellFn);
            _editFloat ->bindInput(InputID::BUTTON_3, "Cancel", cancellFn);
            _editEnum  ->bindInput(InputID::BUTTON_3, "Cancel", cancellFn);
            _editString->bindInput(InputID::BUTTON_3, "Cancel", cancellFn);
        }

        // ------------------------------------------------------------------
        // Settings tree setup
        // ------------------------------------------------------------------

        std::shared_ptr<SettingsState> settingsState() { return _settingsState; }

        // ------------------------------------------------------------------
        // Lifecycle overrides
        // ------------------------------------------------------------------

        void onResume() override
        {
            Window::onResume();
        }

    private:
        std::shared_ptr<SettingsState>   _settingsState;
        std::shared_ptr<EditBoolState>   _editBool;
        std::shared_ptr<EditIntState>    _editInt;
        std::shared_ptr<EditFloatState>  _editFloat;
        std::shared_ptr<EditEnumState>   _editEnum;
        std::shared_ptr<EditStringState> _editString;

    };

} // namespace DisplayModule
