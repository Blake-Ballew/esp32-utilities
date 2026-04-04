#pragma once

#include "Window.hpp"
#include "States/EditStringState.hpp"
#include "TextDrawCommand.hpp"
#include "DisplayUtilities.hpp"
#include "NavigationUtils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // SaveLocationWindow
    // -------------------------------------------------------------------------
    // Prompts the user to enter a name for the current GPS location, then
    // saves it to NavigationUtils::AddSavedLocation().
    //
    // Captures the current GPS coordinates on construction.
    //
    // Input layout:
    //   ENC_UP / ENC_DOWN — cycle characters (handled by EditStringState)
    //   BUTTON_1          — "Del"  (delete last char)
    //   BUTTON_2          — "Done" (confirm + save)
    //   BUTTON_3          — "Back" (cancel, pop window)
    //   BUTTON_4          — "Add"  (append selected character)
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<SaveLocationWindow>());

    class SaveLocationWindow : public Window
    {
    public:
        SaveLocationWindow()
        {
            // Capture current GPS position at the moment the window opens
            NavigationUtils::UpdateGPS();
            _lat = NavigationUtils::GetLocation().lat();
            _lon = NavigationUtils::GetLocation().lng();

            _editState = std::make_shared<EditStringState>();

            // Enter edit state with an empty string and max-length constraint
            StateTransferData initialData;
            {
                auto payload = std::make_shared<ArduinoJson::DynamicJsonDocument>(64);
                (*payload)["cfgVal"] = "";
                (*payload)["maxLen"] = static_cast<int>(STATUS_LENGTH);
                initialData.payload = payload;
            }
            setInitialState(_editState);
            _editState->onEnter(initialData);

            // ENC labels are implicit to EditStringState
            registerInput(InputID::BUTTON_1, "Del");
            registerInput(InputID::BUTTON_4, "Add");

            // BUTTON_3 — cancel
            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [](const InputContext &) { Utilities::popWindow(); });

            // BUTTON_2 — confirm: save the location, show confirmation, pop
            registerInput(InputID::BUTTON_2, "Done");
            addInputCommand(InputID::BUTTON_2,
                [this](const InputContext &)
                {
                    if (_currentState != _editState) return;

                    const std::string &name = _editState->currentString();
                    if (!name.empty())
                    {
                        NavigationUtils::AddSavedLocation({ name, _lat, _lon });
                    }

                    Utilities::popWindow();
                });
        }

    private:
        double _lat = 0.0;
        double _lon = 0.0;
        std::shared_ptr<EditStringState> _editState;
    };

} // namespace DisplayModule
