#pragma once

#include "Window.hpp"
#include "States/SavedLocationsState.hpp"
#include "States/EditStringState.hpp"
#include "States/TrackingState.hpp"
#include "DisplayUtilities.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // EditSavedLocationsWindow
    // -------------------------------------------------------------------------
    // Manage saved GPS locations: scroll, rename, delete, and track.
    //
    // State flow:
    //   SavedLocationsState (list view)
    //     ↓ BUTTON_4 ("Edit")  — push EditStringState to rename selected location
    //     ↓ BUTTON_2 ("Track") — push TrackingState for selected location
    //     ↓ BUTTON_1 ("Del")   — delete selected location (handled in state)
    //     ↓ BUTTON_3 ("Back")  — pop window
    //
    //   EditStringState (rename)
    //     ↓ BUTTON_2 ("Done")  — pop state with result → SavedLocationsState
    //     ↓ BUTTON_3 ("Back")  — pop state without result
    //
    //   TrackingState
    //     ↓ BUTTON_3 ("Back")  — pop state → SavedLocationsState
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<EditSavedLocationsWindow>());

    class EditSavedLocationsWindow : public Window
    {
    public:
        EditSavedLocationsWindow()
        {
            _listState     = std::make_shared<SavedLocationsState>();
            _editStrState  = std::make_shared<EditStringState>();
            _trackingState = std::make_shared<TrackingState>();

            // ----------------------------------------------------------------
            // BUTTON_3 — context-sensitive Back
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [this](const InputContext &ctx)
                {
                    if (_currentState == _listState)
                    {
                        Utilities::popWindow();
                    }
                    else if (_currentState == _editStrState
                             || _currentState == _trackingState)
                    {
                        // Cancel / stop tracking → return to list (no result payload)
                        popState();
                    }
                });

            // ----------------------------------------------------------------
            // BUTTON_4 — Edit selected location name
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_4, "Edit");
            addInputCommand(InputID::BUTTON_4,
                [this](const InputContext &ctx)
                {
                    if (_currentState != _listState) return;
                    if (!_listState->hasLocations()) return;

                    StateTransferData d;
                    d.inputID = ctx.inputID;
                    d.payload = _listState->buildEditPayload();
                    pushState(_editStrState, d);
                });

            // ----------------------------------------------------------------
            // BUTTON_2 — Track selected location
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_2, "Track");
            addInputCommand(InputID::BUTTON_2,
                [this](const InputContext &ctx)
                {
                    if (_currentState != _listState) return;
                    auto payload = _listState->buildTrackPayload();
                    if (!payload) return;

                    StateTransferData d;
                    d.inputID = ctx.inputID;
                    d.payload = payload;
                    pushState(_trackingState, d);
                });

            // ----------------------------------------------------------------
            // BUTTON_1 — Delete (handled directly in SavedLocationsState::handleInput)
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_1, "Del");

            // ----------------------------------------------------------------
            // EditStringState BUTTON_2 confirm — pop with result
            // ----------------------------------------------------------------
            addInputCommand(InputID::BUTTON_2,
                [this](const InputContext &ctx)
                {
                    if (_currentState != _editStrState) return;
                    StateTransferData d;
                    d.inputID = ctx.inputID;
                    d.payload = _editStrState->buildResultPayload();
                    popState(d); // → SavedLocationsState::onEnter sees "return" key
                });

            setInitialState(_listState);
        }

    private:
        std::shared_ptr<SavedLocationsState> _listState;
        std::shared_ptr<EditStringState>     _editStrState;
        std::shared_ptr<TrackingState>       _trackingState;
    };

} // namespace DisplayModule
