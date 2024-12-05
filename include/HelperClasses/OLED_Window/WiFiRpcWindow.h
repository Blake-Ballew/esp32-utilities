#pragma once

#include "OLED_Window.h"
#include "BroadcastTcpState.h"
#include "WiFiTcpStream.h"
#include "NetworkUtils.h"
#include "RpcUtils.h"

namespace
{

    const int RPC_AWAITING_CONNECTION = 0;
    const int RPC_CONNECTION_ESTABLISHED = 1;
    const int RPC_WIFI_FAILED = 2;
}

class WiFiRpcWindow : public OLED_Window
{
public:
    WiFiRpcWindow(OLED_Window *parent) : OLED_Window(parent) 
    {
        _rpcStream = nullptr;
        _broadcast.assignInput(BUTTON_3, ACTION_BACK, "Back");
        _otherState.assignInput(BUTTON_3, ACTION_BACK, "Back");

        auto result = NetworkModule::Utilities::EnableWiFi();
        // Create TCP Stream

        if (result)
        {
            #if DEBUG == 1
            Serial.println("Connected to WiFi!");
            #endif

            _rpcStream = new NetworkModule::WiFiTcpStream(NetworkModule::Utilities::RPC_PORT);

            setInitialState(&_broadcast);
            _rpcStream->SetPort(NetworkModule::Utilities::RPC_PORT);
            _rpcStream->StartServer();

            _rpcStreamID = NetworkModule::Utilities::RegisterNetworkStream(_rpcStream);
            _rpcChannelID = RpcModule::Utilities::AddRpcChannel(1024, 
            NetworkModule::Utilities::RpcRequestHandler,
           NetworkModule::Utilities::RpcResponseHandler);
            NetworkModule::Utilities::AttachRpcStream(_rpcChannelID, _rpcStreamID);

            #if DEBUG == 1
            Serial.print("RPC Channel ID: "); Serial.println(_rpcChannelID);
            Serial.print("RPC Stream ID: "); Serial.println(_rpcStreamID);
            #endif
        }
        else
        {
            setInitialState(&_otherState);
            _rpcConnectionState = RPC_WIFI_FAILED;
        }
    }

    ~WiFiRpcWindow() 
    {
        if (_rpcStream != nullptr)
        {
            delete _rpcStream;
            _rpcStream = nullptr;
        }

        NetworkModule::Utilities::DisableWiFi();
        if (_rpcStreamID != -1)
        {
            NetworkModule::Utilities::UnregisterNetworkStream(_rpcStreamID);
        }

        if (_rpcChannelID != -1)
        {
            RpcModule::Utilities::RemoveRpcChannel(_rpcChannelID);
            NetworkModule::Utilities::DetachRpcStream(_rpcChannelID);
        }
    }

    void drawWindow() 
    {
        OLED_Window::drawWindow();

        if (_rpcConnectionState == RPC_WIFI_FAILED)
        {
            TextFormat prompt;
            prompt.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            prompt.verticalAlignment = ALIGN_CENTER_VERTICAL;
            Display_Utils::printFormattedText("WiFi Error", prompt);
        }
        else if (_rpcConnectionState == RPC_CONNECTION_ESTABLISHED)
        {
            TextFormat prompt;
            prompt.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
            prompt.verticalAlignment = ALIGN_CENTER_VERTICAL;
            Display_Utils::printFormattedText("RPC Client Connected", prompt);
        } 
        else if (_rpcConnectionState == RPC_AWAITING_CONNECTION)
        {
            if (_rpcStream != nullptr && _rpcStream->AcceptClientConnection())
            {
                _rpcConnectionState = RPC_CONNECTION_ESTABLISHED;
                currentState = &_otherState;
                RpcModule::Utilities::EnableRpcChannel(_rpcChannelID);
            }
        }

        Display_Utils::UpdateDisplay().Invoke();
    }

protected:
    // State to broadcast the TCP server
    BroadcastTcpState _broadcast;
    
    // Blank state that does nothing. Just to have something to switch to
    Window_State _otherState;

    NetworkModule::WiFiTcpStream *_rpcStream; 
    int _rpcStreamID = -1;

    int _rpcChannelID = -1;

private:
    int _rpcConnectionState = RPC_AWAITING_CONNECTION;
};