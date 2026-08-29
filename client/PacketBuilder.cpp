#include "PacketBuilder.h"

PacketBuilder::PacketBuilder() {}
std::vector<uint8_t> PacketBuilder::buildPacket(PacketType type, uint32_t messageID, uint32_t sessionID, const std::vector<uint8_t>& body)
{
    PacketHeaderRaw header;
    header.type      = htons(static_cast<uint16_t>(type));
    header.messageID = htonl(messageID);
    header.sessionID = htonl(sessionID);
    header.messageLen = htons(static_cast<uint16_t>(body.size()));

    std::vector<uint8_t> result;
    appendBytes(result, &header, sizeof(header));
    appendBytes(result, body.data(), body.size());
    return result;
}

std::vector<uint8_t> PacketBuilder::buildConnectRequestPacket(uint32_t messageID, uint32_t sessionID)
{
    return buildPacket(PacketType::ConnectRequest, messageID, sessionID);
}
std::vector<uint8_t> PacketBuilder::buildConnectResponsePacket(uint32_t messageID, uint32_t sessionID)
{
    return buildPacket(PacketType::ConnectResponse, messageID, sessionID);
}
std::vector<uint8_t> PacketBuilder::buildRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet)
{
    std::vector<uint8_t> body;
    writeString(body, packet.username);
    writeString(body, packet.password);
    return buildPacket(PacketType::RegisterRequest, messageID, sessionID, body);
}
std::vector<uint8_t> PacketBuilder::buildRegisterResponsePacket(uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket& packet)
{
    std::vector<uint8_t> body;
    body.push_back(packet.success);
    return buildPacket(PacketType::RegisterResponse, messageID, sessionID, body);
}
std::vector<uint8_t> PacketBuilder::buildAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet)
{
    std::vector<uint8_t> body;
    writeString(body, packet.username);
    writeString(body, packet.password);
    return buildPacket(PacketType::AuthRequest, messageID, sessionID, body);
}
std::vector<uint8_t> PacketBuilder::buildAuthResponsePacket(uint32_t messageID, uint32_t sessionID, const AuthResponsePacket& packet)
{
    std::vector<uint8_t> body;
    body.push_back(packet.success);
    return buildPacket(PacketType::AuthResponse, messageID, sessionID, body);
}
std::vector<uint8_t> PacketBuilder::buildMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet)
{
    std::vector<uint8_t> body;
    writeUint32(body, packet.senderID);
    writeUint32(body, packet.chatID);
    writeString(body, packet.text);
    return buildPacket(PacketType::MessageSend, messageID, sessionID, body);
}
std::vector<uint8_t> PacketBuilder::buildDisconnectRequestPacket(uint32_t messageID, uint32_t sessionID)
{
    return buildPacket(PacketType::DisconnectRequest, messageID, sessionID);
}
