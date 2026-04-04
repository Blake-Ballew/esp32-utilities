#pragma once

#include <cstdio>
#include "WindowState.hpp"
#include "DisplayUtilities.hpp"
#include "TextDrawCommand.hpp"
#include "NavigationUtils.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // CompassDebugState
    // -------------------------------------------------------------------------
    // Live compass readout — shows the current azimuth value and, optionally,
    // raw magnetometer values from NavigationUtils.
    //
    // Behaviour:
    //   - refreshIntervalMs() returns REFRESH_RATE_MS so the display auto-ticks.
    //   - Each tick: calls onTick() to read the current azimuth, updates the
    //     draw command text, and re-renders.
    //
    // LED ring feedback (e.g. a compass ring LED pattern) is intentionally
    // omitted here.  Subclass CompassDebugState and override onEnter/onExit/
    // onTick to add LED logic, or wire it in the owning Window.
    //
    // Wiring example (Window):
    //   win.addInputCommand(InputID::BUTTON_3, [&win](auto &) {
    //       win.popState();
    //   });
    //
    // onTick() should be called by the application each time the refresh timer
    // fires (i.e. each time Utilities::render() is invoked while this state is
    // active).

    class CompassDebugState : public WindowState
    {
    public:
        static constexpr uint32_t REFRESH_RATE_MS = 100;

        CompassDebugState()
        {
            bindInput(InputID::BUTTON_3, "Back");
            refreshIntervalMs = REFRESH_RATE_MS;
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &) override
        {
            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Per-tick update
        // Call each time the refresh fires to update the azimuth readout.
        // ------------------------------------------------------------------

        void onTick()
        {
            NavigationUtils::PrintRawValues(); // logs to serial/ESP_LOG
            _rebuildDrawCommands();
        }

        // Returns the most recently read azimuth (degrees, 0–360).
        float currentAzimuth() const { return _azimuth; }

    private:
        float _azimuth = 0.0f;

        void _rebuildDrawCommands()
        {
            _azimuth = NavigationUtils::GetAzimuth();

            clearDrawCommands();

            char buf[24];
            snprintf(buf, sizeof(buf), "Azimuth: %.1f", static_cast<double>(_azimuth));

            addDrawCommand(std::make_shared<TextDrawCommand>(
                std::string(buf),
                TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
            ));
        }
    };

} // namespace DisplayModule
