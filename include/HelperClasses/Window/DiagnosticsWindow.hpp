#pragma once

#include "Window.hpp"
#include "States/DiagnosticsState.hpp"
#include "DisplayUtilities.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // DiagnosticsWindow
    // -------------------------------------------------------------------------
    // Displays live ESP32 system diagnostics (heap, fragmentation, stack).
    // Refreshes at 500 ms while active.
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<DiagnosticsWindow>());

    class DiagnosticsWindow : public Window
    {
    public:
        DiagnosticsWindow()
        {
            _diagState = std::make_shared<DiagnosticsState>();

            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [](const InputContext &) { Utilities::popWindow(); });

            setInitialState(_diagState);
        }

    private:
        std::shared_ptr<DiagnosticsState> _diagState;
    };

} // namespace DisplayModule
