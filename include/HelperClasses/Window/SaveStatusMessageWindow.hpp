#pragma once

#include "Window.hpp"
#include "States/EditStringState.hpp"
#include "DisplayUtilities.hpp"
#include "LoraUtils.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // SaveStatusMessageWindow
    // -------------------------------------------------------------------------
    // Prompts the user to enter a new status message string, then saves it
    // to LoraUtils::AddSavedMessage().
    //
    // Input layout:
    //   ENC_UP / ENC_DOWN — cycle characters
    //   BUTTON_1          — "Del"  (delete last char)
    //   BUTTON_2          — "Done" (confirm + save)
    //   BUTTON_3          — "Back" (cancel, pop window)
    //   BUTTON_4          — "Add"  (append selected character)
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<SaveStatusMessageWindow>());

    class SaveStatusMessageWindow : public Window
    {
    public:
        SaveStatusMessageWindow()
        {
            _editState = std::make_shared<EditStringState>();

            StateTransferData initialData;
            {
                auto payload = std::make_shared<ArduinoJson::DynamicJsonDocument>(64);
                (*payload)["cfgVal"] = "";
                (*payload)["maxLen"] = static_cast<int>(STATUS_LENGTH);
                initialData.payload = payload;
            }
            setInitialState(_editState);
            _editState->onEnter(initialData);

            registerInput(InputID::BUTTON_1, "Del");
            registerInput(InputID::BUTTON_4, "Add");

            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [](const InputContext &) { Utilities::popWindow(); });

            registerInput(InputID::BUTTON_2, "Done");
            addInputCommand(InputID::BUTTON_2,
                [this](const InputContext &)
                {
                    if (_currentState != _editState) return;

                    const std::string &msg = _editState->currentString();
                    if (!msg.empty())
                        LoraUtils::AddSavedMessage(msg);

                    Utilities::popWindow();
                });
        }

    private:
        std::shared_ptr<EditStringState> _editState;
    };

} // namespace DisplayModule
