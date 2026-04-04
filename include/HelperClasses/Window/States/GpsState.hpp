#pragma once

#include <cstdio>
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "NavigationUtils.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // GpsState
    // -------------------------------------------------------------------------
    // Displays current GPS latitude and longitude.
    // Refreshes at 1 Hz via refreshIntervalMs().
    //
    // Shows "GPS Not Connected" when no fix is available.
    //
    // Wiring example (Window):
    //   registerInput(InputID::BUTTON_3, "Back");
    //   addInputCommand(InputID::BUTTON_3, [](auto &) { Utilities::popWindow(); });

    class GpsState : public WindowState
    {
    public:
        static constexpr uint32_t REFRESH_RATE_MS = 1000;

        GpsState()
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
        // Tick — called each refresh cycle to update GPS readings
        // ------------------------------------------------------------------

        void onTick() override
        {
            _rebuildDrawCommands();
        }

    private:
        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            NavigationUtils::UpdateGPS();

            if (NavigationUtils::IsGPSConnected())
            {
                auto loc = NavigationUtils::GetLocation();

                char lat[32], lon[32];
                snprintf(lat, sizeof(lat), "Lat: %.8f", loc.lat());
                snprintf(lon, sizeof(lon), "Lon: %.8f", loc.lng());

                addDrawCommand(std::make_shared<TextDrawCommand>(
                    std::string(lat),
                    TextFormat{ TextAlignH::LEFT, TextAlignV::LINE, 1 }
                ));
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    std::string(lon),
                    TextFormat{ TextAlignH::LEFT, TextAlignV::LINE, 2 }
                ));
            }
            else
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    "GPS Not Connected",
                    TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 2 }
                ));

                std::string satelliteStr = "Satellites: " + std::to_string(NavigationUtils::GetSatelliteCount());
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    satelliteStr,
                    TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 3 }
                ));
            }
        }
    };

} // namespace DisplayModule
