#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H
#include <vector>
#include <unistd.h>
#include "Packets.h"
class PacketBuilder
{
public:
    PacketBuilder();
    static std::vector<uint8_t> buildConnectRequestPacket(uint32_t messageID, uint32_t sessionID);
    static std::vector<uint8_t> buildConnectResponsePacket(uint32_t messageID, uint32_t sessionID);

    static std::vector<uint8_t> buildRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet);
    static std::vector<uint8_t> buildRegisterResponsePacket(uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket& packet);

    static std::vector<uint8_t> buildAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet);
    static std::vector<uint8_t> buildAuthResponsePacket(uint32_t messageID, uint32_t sessionID, const AuthResponsePacket& packet);

    static std::vector<uint8_t> buildMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet);

    static std::vector<uint8_t> buildDisconnectRequestPacket(uint32_t messageID, uint32_t sessionID);


};

#endif // PACKETBUILDER_H
