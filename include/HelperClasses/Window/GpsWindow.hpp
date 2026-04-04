#pragma once

#include "Window.hpp"
#include "States/GpsState.hpp"
#include "DisplayUtilities.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // GpsWindow
    // -------------------------------------------------------------------------
    // Simple window that shows current GPS latitude / longitude.
    // Refreshes at 1 Hz while active; pauses refresh when covered by another
    // window (handled automatically through the queue-timeout mechanism).
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<GpsWindow>());

    class GpsWindow : public Window
    {
    public:
        GpsWindow()
        {
            _gpsState = std::make_shared<GpsState>();

            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [](const InputContext &) { Utilities::popWindow(); });

            setInitialState(_gpsState);
        }

    private:
        std::shared_ptr<GpsState> _gpsState;
    };

} // namespace DisplayModule
