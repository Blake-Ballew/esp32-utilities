#pragma once

#include <string>
#include "WindowState.hpp"
#include "DisplayUtilities.hpp"
#include "TextDrawCommand.hpp"
#include <ArduinoJson.h>

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // ConfirmState
    // -------------------------------------------------------------------------
    // Presents a yes/no prompt and returns the result via StateTransferData.
    //
    // On enter:
    //   If transferData.payload is set and contains "confirmPrompt" (string),
    //   that string is shown as the prompt.  Otherwise the default prompt is
    //   used.
    //
    // On exit (BUTTON_3 = No, BUTTON_4 = Yes):
    //   A fresh DynamicJsonDocument is allocated and placed in
    //   transferData.payload.  It contains:
    //     { "confirmed": true|false }
    //
    //   The caller (Window onInputCommand / adjacent state's onEnter) reads
    //   this from the StateTransferData it receives.
    //
    // Typical wiring in a Window:
    //   win.addInputCommand(InputID::BUTTON_3, [confirmState](auto &ctx) {
    //       StateTransferData d; d.inputID = ctx.inputID;
    //       win.switchState(prevState, d);
    //   });
    //   win.addInputCommand(InputID::BUTTON_4, [confirmState](auto &ctx) {
    //       StateTransferData d; d.inputID = ctx.inputID;
    //       win.switchState(prevState, d);
    //   });

    class ConfirmState : public WindowState
    {
    public:
        // Default prompt shown when none is supplied via payload.
        static constexpr const char *DEFAULT_PROMPT = "Are you sure?";

        ConfirmState()
        {
            bindInput(InputID::BUTTON_3, "No");
            bindInput(InputID::BUTTON_4, "Yes");
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            _confirmed = false;
            _prompt    = DEFAULT_PROMPT;

            if (data.payload)
            {
                auto &doc = *data.payload;
                if (doc.containsKey("confirmPrompt"))
                    _prompt = doc["confirmPrompt"].as<std::string>();
            }

            _rebuildDrawCommands();
        }

        // Build the outgoing payload before the Window transitions away.
        // Call this from the Window's BUTTON_3 / BUTTON_4 onInputCommand
        // to stamp the result into the StateTransferData before switching:
        //
        //   win.addInputCommand(InputID::BUTTON_4, [cs = confirmState](auto &ctx) {
        //       cs->confirm(true);
        //       StateTransferData d;
        //       d.inputID = ctx.inputID;
        //       d.payload = cs->buildResultPayload();
        //       win.switchState(nextState, d);
        //   });

        void confirm(bool value) { _confirmed = value; }

        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildResultPayload() const
        {
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(64);
            (*doc)["confirmed"] = _confirmed;
            return doc;
        }

        bool isConfirmed() const { return _confirmed; }

        // ------------------------------------------------------------------
        // Custom prompt override (for programmatic setup without payload)
        // ------------------------------------------------------------------

        void setPrompt(std::string prompt)
        {
            _prompt = std::move(prompt);
            _rebuildDrawCommands();
        }

    private:
        bool        _confirmed = false;
        std::string _prompt    = DEFAULT_PROMPT;

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            // Prompt text — line 2
            addDrawCommand(std::make_shared<TextDrawCommand>(
                _prompt,
                TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 2 }
            ));

            // "No" hint — bottom left  (label driven by Window, this is
            // optional supplementary text so states are self-documenting)
            // The actual button labels are rendered by WindowLayer from the
            // Window's InputEntry map — nothing to do here for those.
        }
    };

} // namespace DisplayModule
