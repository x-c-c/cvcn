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

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const ConnectRequestData&)
{
    return buildPacket(PacketType::ConnectRequest, messageID, sessionID);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const ConnectResponseData&)
{
    return buildPacket(PacketType::ConnectResponse, messageID, sessionID);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestData& data)
{
    std::vector<uint8_t> body;
    ByteWriter::writeString(body, data.username);
    ByteWriter::writeString(body, data.password);
    return buildPacket(PacketType::RegisterRequest, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const RegisterResponseData& data)
{
    std::vector<uint8_t> body;
    ByteWriter::writeUint8(body, data.success);
    return buildPacket(PacketType::RegisterResponse, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestData& data)
{
    std::vector<uint8_t> body;
    ByteWriter::writeString(body, data.username);
    ByteWriter::writeString(body, data.password);
    return buildPacket(PacketType::AuthRequest, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const AuthResponseData& data)
{
    std::vector<uint8_t> body;
    ByteWriter::writeUint8(body, data.success);
    return buildPacket(PacketType::AuthResponse, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const MessageSendData& data)
{
    std::vector<uint8_t> body;
    ByteWriter::writeUint32BE(body, data.senderID);
    ByteWriter::writeUint32BE(body, data.chatID);
    ByteWriter::writeString(body, data.text);
    return buildPacket(PacketType::MessageSend, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID, const DisconnectRequestData&)
{
    return buildPacket(PacketType::DisconnectRequest, messageID, sessionID);
}
