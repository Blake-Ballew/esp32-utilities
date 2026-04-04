#pragma once

// WindowInterface.hpp
// The minimal interface Utilities depends on for window management.
// Sits below Utilities in the dependency graph — no Utilities include here.
//
// Dependency chain:
//   DisplayTypes.hpp
//       ↓
//   WindowInterface.hpp   ←  Utilities depends on this
//       ↓
//   DisplayUtilities.hpp
//       ↓
//   Window.hpp            ←  concrete implementation

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include "DisplayTypes.hpp"
#include "WindowState.hpp"

namespace DisplayModule
{
    // Forward declarations — keeps this header free of HelperClasses includes.
    // class WindowState;
    class DrawCommand;

    // -------------------------------------------------------------------------
    // WindowInterface
    // -------------------------------------------------------------------------
    // Abstract interface for all windows. Utilities holds a
    // stack<shared_ptr<WindowInterface>> and only ever calls these methods.
    //
    // Concrete windows inherit from Window (which inherits this) and provide
    // full implementations. Custom application windows may also implement this
    // directly if they don't need Window's state machine machinery.

    class WindowInterface
    {
    public:
        // Shared type aliases — used by WindowLayer / ContentLayer so they
        // can work through WindowInterface without depending on Window.
        using DrawCmds = std::vector<std::shared_ptr<DrawCommand>>;
        using InputMap = std::unordered_map<uint8_t, InputEntry>;

        virtual ~WindowInterface() = default;

        // ------------------------------------------------------------------
        // Input dispatch
        // Called by Utilities::handleInput on the active window.
        // ------------------------------------------------------------------
        virtual void handleInput(const InputContext &ctx) = 0;

        // ------------------------------------------------------------------
        // Post-render notification
        // Called by Utilities::render() immediately after all layers draw.
        // Used to fire onRenderedCommands for the last input.
        // ------------------------------------------------------------------
        virtual void handleRendered() = 0;

        // ------------------------------------------------------------------
        // Lifecycle
        // Called by Utilities when this window is pushed onto or popped from
        // the window stack.
        // ------------------------------------------------------------------
        virtual void onPause()  = 0;
        virtual void onResume() = 0;

        // ------------------------------------------------------------------
        // Autonomous refresh
        // Called by Manager after each timeout-driven render (not on input).
        // Used for per-frame logic like cursor blink, countdowns, etc.
        // Default-implemented as no-op so simple windows don't need to override.
        // ------------------------------------------------------------------
        virtual void onTick() {}

        // ------------------------------------------------------------------
        // Refresh interval
        // Read by Utilities::queueTimeout() to set the FreeRTOS queue timeout.
        // Return 0 for input-only redraw (portMAX_DELAY).
        // Return non-zero for autonomous redraw at that interval in ms.
        // ------------------------------------------------------------------
        virtual uint32_t activeStateRefreshInterval() const = 0;

        // ------------------------------------------------------------------
        // Render interface — called by ContentLayer and WindowLayer each frame.
        // Exposed here so layers can work purely through WindowInterface without
        // depending on the concrete Window class.
        // ------------------------------------------------------------------

        // Returns the currently active state (nullptr if none).
        virtual std::shared_ptr<WindowState> currentState() const = 0;

        // Returns window-level chrome draw commands (borders, titles, etc.).
        virtual const DrawCmds &drawCommands() const = 0;

        // Returns the window-level input map (labels + command vectors).
        virtual const InputMap &inputs() const = 0;
    };

} // namespace DisplayModule
