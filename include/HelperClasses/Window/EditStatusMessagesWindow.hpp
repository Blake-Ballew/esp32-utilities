#pragma once

#include "Window.hpp"
#include "States/SavedStatusMsgListState.hpp"
#include "States/EditStringState.hpp"
#include "DisplayUtilities.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // EditStatusMessagesWindow
    // -------------------------------------------------------------------------
    // Manage saved LoRa status messages: scroll, edit, delete, and create.
    //
    // State flow:
    //   SavedStatusMsgListState (list view)
    //     ↓ BUTTON_4 ("Edit")   — push EditStringState pre-filled with current msg
    //     ↓ BUTTON_2 ("Create") — push EditStringState with empty string
    //     ↓ BUTTON_1 ("Del")    — delete selected message (handled in state)
    //     ↓ BUTTON_3 ("Back")   — pop window
    //
    //   EditStringState
    //     ↓ BUTTON_2 ("Done")   — pop state with result
    //     ↓ BUTTON_3 ("Back")   — pop state without result
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<EditStatusMessagesWindow>());

    class EditStatusMessagesWindow : public Window
    {
    public:
        EditStatusMessagesWindow()
        {
            _listState    = std::make_shared<SavedStatusMsgListState>();
            _editStrState = std::make_shared<EditStringState>();

            // ----------------------------------------------------------------
            // BUTTON_3 — context-sensitive Back
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [this](const InputContext &ctx)
                {
                    if (_currentState == _listState)
                        Utilities::popWindow();
                    else
                        popState(); // Cancel edit → return to list
                });

            // ----------------------------------------------------------------
            // BUTTON_4 — Edit existing message
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_4, "Edit");
            addInputCommand(InputID::BUTTON_4,
                [this](const InputContext &ctx)
                {
                    if (_currentState != _listState) return;

                    StateTransferData d;
                    d.inputID = ctx.inputID;
                    d.payload = _listState->buildEditPayload();
                    pushState(_editStrState, d);
                });

            // ----------------------------------------------------------------
            // BUTTON_2 — Create new message (or confirm edit)
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_2, "Create");
            addInputCommand(InputID::BUTTON_2,
                [this](const InputContext &ctx)
                {
                    if (_currentState == _listState)
                    {
                        // Open new-message editor
                        StateTransferData d;
                        d.inputID = ctx.inputID;
                        d.payload = _listState->buildCreatePayload();
                        pushState(_editStrState, d);
                    }
                    else if (_currentState == _editStrState)
                    {
                        // Confirm edit
                        StateTransferData d;
                        d.inputID = ctx.inputID;
                        d.payload = _editStrState->buildResultPayload();
                        popState(d);
                    }
                });

            // ----------------------------------------------------------------
            // BUTTON_1 — Delete (handled directly in SavedStatusMsgListState)
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_1, "Del");

            setInitialState(_listState);
        }

    private:
        std::shared_ptr<SavedStatusMsgListState> _listState;
        std::shared_ptr<EditStringState>         _editStrState;
    };

} // namespace DisplayModule
