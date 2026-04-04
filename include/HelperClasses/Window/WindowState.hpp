#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <cstdint>
#include "DisplayUtilities.hpp"
#include "DrawCommand.hpp"
#include "EventHandler.hpp"
#include <ArduinoJson.h>

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // StateTransferData
    // -------------------------------------------------------------------------
    // Passed between states on entry/exit.
    //
    // payload is a shared_ptr<DynamicJsonDocument> — short-lived heap allocation,
    // ref-counted so no manual delete is needed anywhere. Sending state allocates
    // it on exit; receiving state reads it on enter and lets it drop when done.
    // Both sides can hold a copy of the shared_ptr if needed.

    struct StateTransferData
    {
        uint8_t  inputID  = 0;
        uint32_t actionID = 0; // what triggered the transition
        std::shared_ptr<ArduinoJson::DynamicJsonDocument> payload;
    };

    // -------------------------------------------------------------------------
    // StateInputBinding
    // -------------------------------------------------------------------------
    // Associates a user-visible label and an optional callback action with a
    // physical input ID, from the perspective of a single WindowState.
    //
    // States populate their bindings in onEnter() via WindowState::bindInput()
    // and they are automatically cleared in onExit() so the next state starts
    // with a clean slate.
    //
    // WindowLayer reads these bindings (preferring them over any window-level
    // label) so button hints always reflect what the *current state* will do,
    // not what the window generically does.
    //
    // action may be nullptr — useful when a state only wants to override the
    // displayed label but the actual work is done by the Window's onInputCommand.

    struct StateInputBinding
    {
        std::string                        label;
        EventHandler<const InputContext &> actions; // empty = label-only
    };

    // -------------------------------------------------------------------------
    // WindowState
    // -------------------------------------------------------------------------
    // Base class for all window states.
    //
    // Key design decisions vs. Window_State:
    //   - No raw pointers. Adjacent states held as weak_ptr — they are owned
    //     by the Window and will expire when the window dies.
    //   - No OLED_Content pointer. States own their own drawing logic via
    //     draw commands populated in onEnter().
    //   - refreshIntervalMs() replaces the global timer interval on Utilities.
    //     Return 0 for input-only redraw (portMAX_DELAY queue timeout).
    //   - No display static member. DrawContext is passed in at draw time.
    //   - Input bindings: call bindInput() in onEnter() to declare per-state
    //     button labels and optional callbacks. Bindings are auto-cleared on
    //     onExit() so each state always starts fresh.
    //   - States can still reference siblings via weak_ptr adjacentStates for
    //     transition targets, but they don't own them.

    class WindowState : public std::enable_shared_from_this<WindowState>
    {
    public:
        using DrawCmds    = std::vector<std::shared_ptr<DrawCommand>>;
        using BindingMap  = std::unordered_map<uint8_t, StateInputBinding>;

        WindowState() = default;
        virtual ~WindowState() = default;

        WindowState(const WindowState &) = delete;
        WindowState &operator=(const WindowState &) = delete;

        size_t refreshIntervalMs = 0;

        // ------------------------------------------------------------------
        // Adjacent state registration
        // Used by Window to wire up state transitions.
        // Stored as weak_ptr — Window owns the states, not the states themselves.
        // ------------------------------------------------------------------
        void setAdjacentState(uint8_t inputID, std::weak_ptr<WindowState> state)
        {
            _adjacentStates[inputID] = std::move(state);
        }

        void clearAdjacentState(uint8_t inputID)
        {
            _adjacentStates.erase(inputID);
        }

        std::shared_ptr<WindowState> getAdjacentState(uint8_t inputID) const
        {
            auto it = _adjacentStates.find(inputID);
            if (it == _adjacentStates.end()) return nullptr;
            return it->second.lock(); // nullptr if the window has already died
        }

        bool hasAdjacentState(uint8_t inputID) const
        {
            auto it = _adjacentStates.find(inputID);
            if (it == _adjacentStates.end()) return false;
            return !it->second.expired();
        }

        // ------------------------------------------------------------------
        // Draw commands
        // Populated in onEnter(), cleared automatically in onExit().
        // Read by ContentLayer — not called directly by Window.
        // ------------------------------------------------------------------
        void addDrawCommand(std::shared_ptr<DrawCommand> cmd)
        {
            _drawCommands.push_back(std::move(cmd));
        }

        void clearDrawCommands() { _drawCommands.clear(); }

        const DrawCmds &drawCommands() const { return _drawCommands; }

        void bindInput(uint8_t inputID,
                       std::string label,
                       std::function<void(const InputContext &)> action = nullptr)
        {
            ESP_LOGV(TAG, "Binding input %d to label '%s'%s", inputID, label.c_str(),
                     action ? " with action" : " with no action");

            auto &binding  = _inputBindings[inputID];
            binding.label  = std::move(label);
            if (action)
            {
                binding.actions += std::move(action);
            } 
        }

        void bindInput(uint8_t inputID,
                       std::function<void(const InputContext &)> action)
        {
            auto &binding  = _inputBindings[inputID];
            if (action) 
            {
                binding.actions += std::move(action);
            }
        }

        // Append an action without replacing the label or existing actions.
        void addInputAction(uint8_t inputID,
                            std::function<void(const InputContext &)> action)
        {
            if (action) _inputBindings[inputID].actions += std::move(action);
        }

        void unbindInput(uint8_t inputID)
        {
            _inputBindings.erase(inputID);
        }

        void clearInputBindings()
        {
            _inputBindings.clear();
        }

        // Returns the binding for an input, or nullptr if not registered.
        const StateInputBinding *inputBinding(uint8_t inputID) const
        {
            auto it = _inputBindings.find(inputID);
            return (it != _inputBindings.end()) ? &it->second : nullptr;
        }

        // Read-only access for WindowLayer label rendering.
        const BindingMap &inputBindings() const { return _inputBindings; }

        // Called by Window::handleInput() — invokes all registered actions.
        void dispatchInputBinding(uint8_t inputID, const InputContext &ctx)
        {
            auto it = _inputBindings.find(inputID);
            if (it != _inputBindings.end())
                it->second.actions(ctx);
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        // Called when this state becomes active.
        // transferData describes what triggered the transition.
        // Override to populate draw commands and input bindings.
        virtual void onEnter(const StateTransferData &data) {}

        // Called when this state is leaving.
        // Clears draw commands automatically — override for additional
        // teardown, but call WindowState::onExit() first.
        // Input bindings are NOT cleared: constructor-registered actions are
        // permanent and must not be destroyed on every state transition.
        virtual void onExit()
        {
            clearDrawCommands();
        }

        // Called when the window is paused (e.g. another window pushed on top)
        virtual void onPause() {}

        // Called when the window resumes (e.g. top window popped)
        virtual void onResume() {}

        // ------------------------------------------------------------------
        // Exit data
        // Called by the Window before transitioning away from this state so
        // the state can populate the payload that the next state will receive
        // in onEnter().  Override to encode outgoing state into the document.
        // The window sets data.inputID to the triggering input before calling.
        // ------------------------------------------------------------------
        virtual StateTransferData buildExitData(uint8_t /*inputID*/) { return {}; }

        // ------------------------------------------------------------------
        // Autonomous tick
        // Called by Window::onTick() each time the refresh timer fires
        // while this state is active.  Override to animate, update readings,
        // etc.  Default is a no-op.
        // ------------------------------------------------------------------
        virtual void onTick() {}

        // ------------------------------------------------------------------
        // Interrupt guard
        // Hints to the system that this state should not be interrupted
        // by unexpected window changes (e.g. mid-text-entry).
        // ------------------------------------------------------------------
        bool allowsInterrupts() const { return _allowInterrupts; }

    protected:
        bool _allowInterrupts = true;

        DrawCmds   _drawCommands;
        BindingMap _inputBindings;

        // Adjacent states — weak to avoid circular ownership.
        // Window owns the shared_ptrs; states only hold non-owning references.
        std::unordered_map<uint8_t, std::weak_ptr<WindowState>> _adjacentStates;
    };

} // namespace DisplayModule
