#pragma once
#include "Packets.h"
#include <cstdint>
#include <vector>

class PacketParser
{
public:
    PacketParser();

    static bool deserializeHeader(const std::vector<uint8_t>& rawData, PacketHeaderRaw& header);

    static bool parseConnectRequestPacket(const std::vector<uint8_t>& body, ConnectRequestPacket& packet);
    static bool parseConnectResponsePacket(const std::vector<uint8_t>& body, ConnectResponsePacket& packet);

    static bool parseRegisterRequestPacket(const std::vector<uint8_t>& body, RegisterRequestPacket& packet);
    static bool parseRegisterResponsePacket(const std::vector<uint8_t>& body, RegisterResponsePacket& packet);

    static bool parseAuthRequestPacket(const std::vector<uint8_t>& body, AuthRequestPacket& packet);
    static bool parseAuthResponsePacket(const std::vector<uint8_t>& body, AuthResponsePacket& packet);

    static bool parseMessageSendPacket(const std::vector<uint8_t>& body, MessageSendPacket& packet);

    static bool parseDisconnectRequestPacket(const std::vector<uint8_t>& body, DisconnectRequestPacket& packet);
};
