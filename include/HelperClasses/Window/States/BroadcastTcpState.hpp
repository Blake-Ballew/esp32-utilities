#pragma once

#include <string>
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "IpUtils.h"
#include "TcpConnectionBroadcast.h"
#include "esp_log.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // BroadcastTcpState
    // -------------------------------------------------------------------------
    // Establishes a UDP broadcast announcing the device's TCP RPC server address
    // and port, then refreshes the display every 500 ms to keep the broadcast
    // alive and show the current IP.
    //
    // The owning Window is responsible for wiring BUTTON_3 (Back).

    class BroadcastTcpState : public WindowState
    {
    public:
        static constexpr uint32_t REFRESH_RATE_MS = 500;
        static constexpr const char *TAG = "BroadcastTcpState";

        BroadcastTcpState()
        {
            bindInput(InputID::BUTTON_3, "Back");
            refreshIntervalMs = REFRESH_RATE_MS;
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &) override
        {
            if (!_broadcast.EstablishConnection(
                    ConnectivityModule::IpUtils::GetWiFiBroadcastIP(),
                    _broadcast.PortToBroadcast()))
            {
                ESP_LOGE(TAG, "Failed to establish broadcast connection");
            }
            else
            {
                ESP_LOGI(TAG, "Broadcast connection established");
            }

            _broadcast.SetPortToBroadcast(ConnectivityModule::IpUtils::RPC_PORT);
            _broadcast.SendBroadcast();

            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Tick — re-broadcast and refresh display
        // ------------------------------------------------------------------

        void onTick() override
        {
            _broadcast.SendBroadcast();
            _rebuildDrawCommands();
        }

    private:
        ConnectivityModule::TcpConnectionBroadcast _broadcast;

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            std::string ip   = WiFi.localIP().toString().c_str();
            std::string port = std::to_string(ConnectivityModule::IpUtils::RPC_PORT);

            addDrawCommand(std::make_shared<TextDrawCommand>(
                "TCP Server on:",
                TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 1 }
            ));
            addDrawCommand(std::make_shared<TextDrawCommand>(
                "IP: " + ip,
                TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 2 }
            ));
            addDrawCommand(std::make_shared<TextDrawCommand>(
                "Port: " + port,
                TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 3 }
            ));
        }
    };

} // namespace DisplayModule
