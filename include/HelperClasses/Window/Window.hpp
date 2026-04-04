#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <stack>
#include <unordered_map>
#include <string>
#include <cstdint>
#include "Interfaces/DisplayTypes.hpp"
#include "Interfaces/WindowInterface.hpp"
#include "DisplayUtilities.hpp"
#include "WindowState.hpp"
#include "HelperClasses/DrawCommands/DrawCommand.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // Window
    // -------------------------------------------------------------------------
    // Concrete base class for all display windows.
    // Inherits WindowInterface so Utilities can manage it through the interface.
    //
    // Key design decisions:
    //   - No parent pointer. Windows have no knowledge of the stack.
    //   - States held as shared_ptr; state stack is a std::stack<shared_ptr>.
    //   - State-to-state references use weak_ptr (see WindowState.hpp).
    //   - Per-input InputEntry replaces the old callbackID map.
    //   - _drawCommands — persistent window-level chrome, read by WindowLayer.
    //   - Drawing driven entirely by the layer pipeline in Utilities::render().
    //   - No OLED_ prefix, no Adafruit_SSD1306 dependency (uses DrawContext).
    //
    // InputEntry, DrawCmds, and InputMap are defined in DisplayTypes.hpp /
    // WindowInterface and inherited — no redefinition needed here.

    class Window : public WindowInterface,
                   public std::enable_shared_from_this<Window>
    {
    public:
        using CommandFn  = InputEntry::CommandFn;
        using CommandVec = InputEntry::CommandVec;
        using InputMap   = WindowInterface::InputMap;
        using DrawCmds   = WindowInterface::DrawCmds;

        Window() = default;
        virtual ~Window() = default;

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        // ------------------------------------------------------------------
        // State management
        // ------------------------------------------------------------------

        void setInitialState(std::shared_ptr<WindowState> state)
        {
            _currentState = state;
            if (_currentState)
                _currentState->onEnter({});
        }

        std::shared_ptr<WindowState> currentState() const override { return _currentState; }

        void switchState(std::shared_ptr<WindowState> next,
                         StateTransferData data = {})
        {
            _transitionTo(next, false, std::move(data));
        }

        void pushState(std::shared_ptr<WindowState> next,
                       StateTransferData data = {})
        {
            _transitionTo(next, true, std::move(data));
        }

        void popState(StateTransferData data = {})
        {
            if (_stateStack.empty()) return;
            auto prev = _stateStack.top();
            _stateStack.pop();
            ESP_LOGI(TAG, "Popping state");
            _transitionTo(prev, false, std::move(data));
        }

        bool hasStateHistory() const { return !_stateStack.empty(); }

        // ------------------------------------------------------------------
        // Input registration
        // ------------------------------------------------------------------

        void registerInput(uint8_t inputID, std::string label)
        {
            _inputs[inputID].label = std::move(label);
        }

        void addInputCommand(uint8_t inputID, CommandFn fn)
        {
            _inputs[inputID].onInputCommands.push_back(std::move(fn));
        }

        void addRenderedCommand(uint8_t inputID, CommandFn fn)
        {
            _inputs[inputID].onRenderedCommands.push_back(std::move(fn));
        }

        void clearInput(uint8_t inputID)
        {
            _inputs.erase(inputID);
        }

        const InputMap &inputs() const override { return _inputs; }

        // ------------------------------------------------------------------
        // Window-level draw commands (persistent chrome, read by WindowLayer)
        // ------------------------------------------------------------------

        void addDrawCommand(std::shared_ptr<DrawCommand> cmd)
        {
            _drawCommands.push_back(std::move(cmd));
        }

        void clearDrawCommands() { _drawCommands.clear(); }

        const DrawCmds &drawCommands() const override { return _drawCommands; }

        // ------------------------------------------------------------------
        // WindowInterface implementation
        // ------------------------------------------------------------------

        void handleInput(const InputContext &ctx) override
        {
            // Snapshot the active state before running window-level commands.
            // A command may trigger a state transition (e.g. pushState); if it
            // does we must NOT dispatch the triggering input to the new state —
            // the new state already received its setup via onEnter() and should
            // not also process the input that caused the transition.
            auto stateAtInputTime = _currentState;

            // 1. Window-level commands — navigation, side-effects that belong
            //    to the window regardless of which state is active.
            auto it = _inputs.find(ctx.inputID);
            if (it != _inputs.end())
                _runCommands(it->second.onInputCommands, ctx);

            // Only dispatch to the state that was active when the input arrived.
            if (stateAtInputTime && stateAtInputTime == _currentState)
            {
                ESP_LOGI(TAG, "Dispatching input to active state: inputID = %d", ctx.inputID);
                // 2. Active state's binding actions — invokes all callbacks
                //    registered via bindInput() / addInputAction() in onEnter().
                _currentState->dispatchInputBinding(ctx.inputID, ctx);
            }

            _pendingRenderedInput = ctx;
            _hasPendingRendered   = true;
        }

        void handleRendered() override
        {
            if (!_hasPendingRendered) return;

            auto it = _inputs.find(_pendingRenderedInput.inputID);
            if (it != _inputs.end())
                _runCommands(it->second.onRenderedCommands, _pendingRenderedInput);

            _hasPendingRendered = false;
        }

        void onResume() override
        {
            if (_currentState)
                _currentState->onResume();
        }

        void onPause() override
        {
            if (_currentState)
                _currentState->onPause();
        }

        void onTick() override
        {
            if (_currentState)
                _currentState->onTick();
        }

        uint32_t activeStateRefreshInterval() const override
        {
            if (!_currentState) return 0;
            return _currentState->refreshIntervalMs;
        }

    protected:
        std::shared_ptr<WindowState>             _currentState;
        std::stack<std::shared_ptr<WindowState>> _stateStack;

        InputMap _inputs;
        DrawCmds _drawCommands;

        InputContext _pendingRenderedInput;
        bool         _hasPendingRendered = false;

        void _transitionTo(std::shared_ptr<WindowState> next,
                           bool pushCurrent,
                           StateTransferData data = {})
        {
            if (!next) return;

            if (_currentState)
            {
                if (pushCurrent)
                    _stateStack.push(_currentState);
                _currentState->onExit();
            }

            _currentState = next;
            _currentState->onEnter(data);
        }

        void _runCommands(const CommandVec &cmds, const InputContext &ctx)
        {
            auto stateBefore = _currentState;
            for (auto &fn : cmds)
            {
                fn(ctx);
                if (_currentState != stateBefore)
                    break; // a command triggered a state transition; stop here
            }
        }
    };

} // namespace DisplayModule
