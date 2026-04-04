#pragma once

#include <string>
#include <cstdio>
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "NavigationUtils.h"
#include "LED_Utils.h"
#include "RingPoint.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // TrackingState
    // -------------------------------------------------------------------------
    // Tracks a GPS target coordinate and drives a RingPoint LED pattern to
    // indicate bearing and distance.
    //
    // Payload in:
    //   {
    //     "lat":     <double>   — target latitude
    //     "lon":     <double>   — target longitude
    //     "color_R": <uint8_t>  — LED red component   (optional, default 255)
    //     "color_G": <uint8_t>  — LED green component (optional, default 255)
    //     "color_B": <uint8_t>  — LED blue component  (optional, default 255)
    //     "displayTxt": ["line1", ...]  — header lines to show (optional)
    //   }
    //
    // Refreshes at 100 ms; _allowInterrupts is false (prevents push-on-top).
    //
    // BUTTON_3 (Back) should be wired by the owning Window.

    class TrackingState : public WindowState
    {
    public:
        static constexpr uint32_t REFRESH_RATE_MS = 100;

        TrackingState()
        {
            _allowInterrupts = false;
            bindInput(InputID::BUTTON_3, "Back");
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            _targetLat = 0.0;
            _targetLon = 0.0;
            _colorR    = 255;
            _colorG    = 255;
            _colorB    = 255;
            _headerLines.clear();

            refreshIntervalMs = REFRESH_RATE_MS;

            if (data.payload)
            {
                auto &doc = *data.payload;
                if (doc.containsKey("lat")) _targetLat = doc["lat"].as<double>();
                if (doc.containsKey("lon")) _targetLon = doc["lon"].as<double>();
                if (doc.containsKey("color_R")) _colorR = doc["color_R"].as<uint8_t>();
                if (doc.containsKey("color_G")) _colorG = doc["color_G"].as<uint8_t>();
                if (doc.containsKey("color_B")) _colorB = doc["color_B"].as<uint8_t>();

                if (doc.containsKey("displayTxt") && doc["displayTxt"].is<ArduinoJson::JsonArray>())
                {
                    for (auto txt : doc["displayTxt"].as<ArduinoJson::JsonArrayConst>())
                        _headerLines.push_back(txt.as<std::string>());
                }
            }

            // Register and configure RingPoint LED pattern
            _ringPointID = RingPoint::RegisteredPatternID();
            if (_ringPointID < 0)
            {
                auto *rp = new RingPoint();
                _ringPointID = LED_Utils::registerPattern(rp);
            }

            LED_Utils::enablePattern(_ringPointID);

            _rebuildDrawCommands();
        }

        void onExit() override
        {
            if (_ringPointID >= 0)
            {
                LED_Utils::disablePattern(_ringPointID);
                LED_Utils::clearPattern(_ringPointID);
            }
            WindowState::onExit();
        }

        void onPause() override
        {
            if (_ringPointID >= 0)
                LED_Utils::disablePattern(_ringPointID);
        }

        void onResume() override
        {
            if (_ringPointID >= 0)
                LED_Utils::enablePattern(_ringPointID);
        }

        // ------------------------------------------------------------------
        // Tick — update GPS and LED
        // ------------------------------------------------------------------

        void onTick() override
        {
            NavigationUtils::UpdateGPS();

            if (_ringPointID >= 0)
            {
                auto currentLoc = NavigationUtils::GetLocation();
                double bearing  = NavigationUtils::GetHeading(currentLoc, _targetLat, _targetLon);
                double distance = NavigationUtils::GetDistance(currentLoc, _targetLat, _targetLon);

                ArduinoJson::StaticJsonDocument<128> cfg;
                cfg["bearing"]  = bearing;
                cfg["distance"] = distance;
                cfg["color_R"]  = _colorR;
                cfg["color_G"]  = _colorG;
                cfg["color_B"]  = _colorB;

                LED_Utils::configurePattern(_ringPointID, cfg);
                LED_Utils::iteratePattern(_ringPointID);
            }

            _rebuildDrawCommands();
        }

    private:
        double _targetLat = 0.0;
        double _targetLon = 0.0;
        uint8_t _colorR = 255, _colorG = 255, _colorB = 255;
        std::vector<std::string> _headerLines;
        int _ringPointID = -1;

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            // Header lines from payload (e.g. location name, sender name)
            uint8_t line = 1;
            for (auto &hl : _headerLines)
            {
                if (line > 3) break;
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    hl,
                    TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, line }
                ));
                ++line;
            }

            // Current distance
            auto currentLoc = NavigationUtils::GetLocation();
            double distance = NavigationUtils::GetDistance(currentLoc, _targetLat, _targetLon);

            char distBuf[24];
            snprintf(distBuf, sizeof(distBuf), "Dist: %.1f m",
                     static_cast<double>(distance));

            addDrawCommand(std::make_shared<TextDrawCommand>(
                std::string(distBuf),
                TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, static_cast<uint8_t>(line) }
            ));
        }
    };

} // namespace DisplayModule
