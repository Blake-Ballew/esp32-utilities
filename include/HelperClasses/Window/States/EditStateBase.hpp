#pragma once

#include <functional>
#include <memory>
#include <ArduinoJson.h>
#include "WindowState.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // EditStateBase
    // -------------------------------------------------------------------------
    // Common base for all edit states (EditBoolState, EditIntState, ...).
    //
    // Provides a confirm-action callback that the owning Window installs once
    // at construction time.  When the user confirms an edit the state fires the
    // callback with a StateTransferData containing its result payload — the
    // callback typically calls Window::popState(data).
    //
    // This keeps edit states unaware of the Window type: the Window defines a
    // lambda that captures itself and hands it to the state via setConfirmAction().
    //
    // Pattern (SettingsWindow constructor):
    //   auto confirm = [this](StateTransferData d){ popState(d); };
    //   _editBool->setConfirmAction(confirm);
    //   _editInt ->setConfirmAction(confirm);
    //   // ...
    //
    // wireConfirmInput() is designed to be called from each derived state's
    // CONSTRUCTOR (not onEnter).  The lambda captures `this` and checks
    // _confirmAction at dispatch time, so it is safe to call before
    // setConfirmAction() has been called — the action will simply be a no-op
    // until the owning Window installs it.
    //
    // Pattern (each edit state's constructor):
    //   wireConfirmInput(InputID::BUTTON_4, "Save");
    //   wireConfirmInput(InputID::BUTTON_2, "Save");   // e.g. EditStringState

    class EditStateBase : public WindowState
    {
    public:
        using ConfirmAction = std::function<void(StateTransferData)>;

        void setConfirmAction(ConfirmAction fn) { _confirmAction = std::move(fn); }

        // Override in each derived state to return the result payload.
        virtual std::shared_ptr<ArduinoJson::DynamicJsonDocument>
        buildResultPayload() const { return nullptr; }

    protected:
        // Call once from the derived state's CONSTRUCTOR for whichever input
        // should confirm the edit.  Binds the label and appends an action that
        // builds the result payload and fires _confirmAction when dispatched.
        // Safe to call before setConfirmAction() — the lambda guards on it.
        void wireConfirmInput(uint8_t inputID, std::string label)
        {
            bindInput(inputID, std::move(label),
                [this](const InputContext &ctx)
                {
                    if (_confirmAction)
                    {
                        StateTransferData d;
                        d.inputID = ctx.inputID;
                        d.payload = buildResultPayload();
                        _confirmAction(std::move(d));
                    }
                });
        }

    private:
        ConfirmAction _confirmAction;
    };

} // namespace DisplayModule
