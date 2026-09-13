#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H
#include <vector>
#include <cstdint>
#include "PacketData.h"

class PacketBuilder
{
private:
    static std::vector<uint8_t> buildPacket(PacketType type, uint32_t messageID, uint32_t sessionID, const std::vector<uint8_t>& body = {});

public:
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const ConnectRequestData&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const ConnectResponseData&);

    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestData&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const RegisterResponseData&);

    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestData&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const AuthResponseData&);

    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const MessageSendData&);

    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const DisconnectRequestData&);

    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const DeleteRequestData&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const DeleteResponseData&);

    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const FindUserRequestData&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const FindUserResponseData&);

    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const CreateChatRequestData&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const CreateChatResponseData&);

    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const ChatListRequestData&);
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID, const ChatListResponseData&);
};

#endif // PACKETBUILDER_H
