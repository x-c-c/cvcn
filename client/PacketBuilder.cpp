#include "PacketBuilder.h"
#include "ByteWriter.h"
#include <cstring>
#include <arpa/inet.h>

std::vector<uint8_t> PacketBuilder::buildPacket(PacketType type, uint32_t messageID, uint32_t sessionID, const std::vector<uint8_t>& body)
{
    PacketHeaderRaw header;
    header.type = htons(static_cast<uint16_t>(type));
    header.messageID = htonl(messageID);
    header.sessionID = htonl(sessionID);
    header.messageLen = htons(static_cast<uint16_t>(body.size()));

    std::vector<uint8_t> result;
    ByteWriter::appendBytes(result, &header, sizeof(header));
    ByteWriter::appendBytes(result, body.data(), body.size());
    return result;
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const ConnectRequestPacket&)
{
    return buildPacket(PacketType::ConnectRequest, messageID, sessionID);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const ConnectResponsePacket&)
{
    return buildPacket(PacketType::ConnectResponse, messageID, sessionID);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet)
{
    std::vector<uint8_t> body;
    ByteWriter::writeString(body, packet.username);
    ByteWriter::writeString(body, packet.password);
    return buildPacket(PacketType::RegisterRequest, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket& packet)
{
    std::vector<uint8_t> body;
    body.push_back(packet.success);
    return buildPacket(PacketType::RegisterResponse, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet)
{
    std::vector<uint8_t> body;
    ByteWriter::writeString(body, packet.username);
    ByteWriter::writeString(body, packet.password);
    return buildPacket(PacketType::AuthRequest, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const AuthResponsePacket& packet)
{
    std::vector<uint8_t> body;
    body.push_back(packet.success);
    return buildPacket(PacketType::AuthResponse, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet)
{
    std::vector<uint8_t> body;
    ByteWriter::writeUint32BE(body, packet.senderID);
    ByteWriter::writeUint32BE(body, packet.chatID);
    ByteWriter::writeString(body, packet.text);
    return buildPacket(PacketType::MessageSend, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const DisconnectRequestPacket&)
{
    return buildPacket(PacketType::DisconnectRequest, messageID, sessionID);
}
