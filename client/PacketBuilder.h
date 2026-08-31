#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H
#include <vector>
#include <cstdint>
#include "Packets.h"
class PacketBuilder
{
private:
    static std::vector<uint8_t> buildPacket(PacketType type, uint32_t messageID, uint32_t sessionID, const std::vector<uint8_t>& body = {});

public:
    PacketBuilder();
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const ConnectRequestPacket&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const ConnectResponsePacket&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const AuthResponsePacket&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const DisconnectRequestPacket&);


    /*
    static std::vector<uint8_t> buildConnectRequestPacket(uint32_t messageID, uint32_t sessionID);
    static std::vector<uint8_t> buildConnectResponsePacket(uint32_t messageID, uint32_t sessionID);

    static std::vector<uint8_t> buildRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet);
    static std::vector<uint8_t> buildRegisterResponsePacket(uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket& packet);

    static std::vector<uint8_t> buildAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet);
    static std::vector<uint8_t> buildAuthResponsePacket(uint32_t messageID, uint32_t sessionID, const AuthResponsePacket& packet);

    static std::vector<uint8_t> buildMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet);

    static std::vector<uint8_t> buildDisconnectRequestPacket(uint32_t messageID, uint32_t sessionID);
*/

};

#endif // PACKETBUILDER_H
