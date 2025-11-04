#pragma once

#include "System_Utils.h"
#include "NetworkStreamInterface.h"
#include <WiFi.h>
#include <unordered_map>

namespace ConnectivityModule
{


    class IpUtils
    {
    public:

        const static int RPC_PORT = 14589;

        // IP
        static IPAddress GetLocalWiFiIP()
        {
            return WiFi.localIP();
        }

        static IPAddress GetWiFiBroadcastIP()
        {
            return WiFi.broadcastIP();
        }

        // Network Streams

        static int RegisterNetworkStream(NetworkStreamInterface *stream)
        {
            int id = _NextNetworkStreamID();
            _NetworkStreams()[id] = stream;
            return id;
        }

        static void UnregisterNetworkStream(int id)
        {
            if (_NetworkStreams().find(id) != _NetworkStreams().end())
            {
                _NetworkStreams().erase(id);
            }
        }

        static NetworkStreamInterface *GetNetworkStream(int id)
        {
            return _NetworkStreams()[id];
        }

        // RPC

        static bool RpcRequestHandler(int channelID, JsonDocument &payload)
        {
            #if DEBUG == 1
            // Serial.print("NetworkUtils::RpcRequestHandler. channelID: "); 
            // Serial.println(channelID);
            #endif

            if (_RpcStreams().find(channelID) != _RpcStreams().end() &&
                _NetworkStreams().find(_RpcStreams()[channelID]) != _NetworkStreams().end())
            {
                auto networkStreamID = _RpcStreams()[channelID];
                auto result = deserializeMsgPack(payload, _NetworkStreams()[networkStreamID]->GetStream());

                if (result == DeserializationError::Ok)
                {
                    #if DEBUG == 1
                    Serial.println("NetworkUtils::RpcRequestHandler. result: Ok");
                    #endif
                    return true;
                }
            }

            return false;
        }

        static void RpcResponseHandler(int channelID, JsonDocument &payload)
        {
            if (_RpcStreams().find(channelID) != _RpcStreams().end() &&
                _NetworkStreams().find(_RpcStreams()[channelID]) != _NetworkStreams().end())
            {
                auto networkStreamID = _RpcStreams()[channelID];
                auto stream = _NetworkStreams()[networkStreamID];

                #if DEBUG == 1
                Serial.println("NetworkUtils::RpcResponseHandler. sening payload: ");
                serializeJson(payload, Serial);
                Serial.println("");
                #endif

                stream->BeginPacket();
                auto result = serializeMsgPack(payload, stream->GetStream());
                #if DEBUG == 1
                Serial.print("NetworkUtils::RpcResponseHandler. bytes written: ");
                Serial.println(result);
                Serial.println("Bytes: ");
                uint8_t buffer[measureMsgPack(payload)];
                serializeMsgPack(payload, buffer, sizeof(buffer));
                for (int i = 0; i < result; i++)
                {
                    Serial.print(buffer[i], HEX);
                    Serial.print(" ");
                }
                Serial.println("");
                #endif
                stream->EndPacket();
            }
        }

        static void AttachRpcStream(int rpcChannelID, int networkStreamID)
        {
            if (_RpcStreams().find(rpcChannelID) == _RpcStreams().end() &&
                _NetworkStreams().find(networkStreamID) != _NetworkStreams().end())
            {
                _RpcStreams()[rpcChannelID] = networkStreamID;
            }
        }

        static void DetachRpcStream(int rpcChannelID)
        {
            if (_RpcStreams().find(rpcChannelID) != _RpcStreams().end())
            {
                _RpcStreams().erase(rpcChannelID);
            }
        }

    private:
        static std::unordered_map<int, NetworkStreamInterface *> &_NetworkStreams()
        {
            static std::unordered_map<int, NetworkStreamInterface *> _NetworkStreams;
            return _NetworkStreams;
        }

        // Map RPC channel IDs to NetworkStream IDs
        static std::unordered_map<int, int> _RpcStreams()
        {
            static std::unordered_map<int, int> _RpcStreams;
            return _RpcStreams;
        }

        static int _NextNetworkStreamID()
        {
            static int _NextNetworkStreamID = 0;
            return _NextNetworkStreamID++;  
        }
    };
}