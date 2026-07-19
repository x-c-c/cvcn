#include "Serializer.h"
#include <cstring>
#include <arpa/inet.h>
/* === Сериализация разных типов данных в вектор uint8_t ===*/
void Serializer::appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count)
{
	const size_t offset = dest.size();
	dest.resize(offset + count);
	std::memcpy(dest.data() + offset, src, count);
}
void Serializer::writeUint16(std::vector<uint8_t>& buf, uint16_t val)
{
	const uint16_t net = htons(val);
	appendBytes(buf, &net, sizeof(net));
}
void Serializer::writeUint32(std::vector<uint8_t>& buf, uint32_t val)
{
	const uint32_t net = htonl(val);
	appendBytes(buf, &net, sizeof(net));
}
void Serializer::writeString(std::vector<uint8_t>& buf, const std::string& str)
{
	const uint16_t len = static_cast<uint16_t>(str.size());
	writeUint16(buf, len);
	appendBytes(buf, str.data(), len);
}

std::vector<uint8_t> Serializer::buildPacket(PacketType type, uint32_t messageID, uint32_t sessionID, const std::vector<uint8_t>& body)
{
	PacketHeaderRaw header;
	header.type_		= htons(static_cast<uint16_t>(type));
	header.messageID_	= htonl(messageID);
	header.sessionID_	= htonl(sessionID);
	header.messageLen_	= htons(static_cast<uint16_t>(body.size()));

	std::vector<uint8_t> result;
	appendBytes(result, &header, sizeof(header));
	appendBytes(result, body.data(), body.size());
	return result;
}

std::vector<uint8_t> Serializer::buildConnectRequestPacket(uint32_t messageID, uint32_t sessionID)
{
	return buildPacket(PacketType::ConnectRequest, messageID, sessionID);
}

std::vector<uint8_t> Serializer::buildConnectResponsePacket(uint32_t messageID, uint32_t sessionID)
{
	return buildPacket(PacketType::ConnectResponse, messageID, sessionID);
}

std::vector<uint8_t> Serializer::buildRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet)
{
	std::vector<uint8_t> body;
	writeString(body, packet.username);
	writeString(body, packet.password);
	return buildPacket(PacketType::RegisterRequest, messageID, sessionID, body);
}

std::vector<uint8_t> Serializer::buildRegisterResponsePacket(uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket& packet)
{
	std::vector<uint8_t> body;
	body.push_back(packet.success);
	return buildPacket(PacketType::RegisterResponse, messageID, sessionID, body);
}

std::vector<uint8_t> Serializer::buildAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet)
{
	std::vector<uint8_t> body;
	writeString(body, packet.username);
	writeString(body, packet.password);
	return buildPacket(PacketType::AuthRequest, messageID, sessionID, body);
}

std::vector<uint8_t> Serializer::buildAuthResponsePacket(uint32_t messageID, uint32_t sessionID, const AuthResponsePacket& packet)
{
	std::vector<uint8_t> body;
	body.push_back(packet.success);
	return buildPacket(PacketType::AuthResponse, messageID, sessionID, body);
}

std::vector<uint8_t> Serializer::buildMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet)
{
	std::vector<uint8_t> body;
	writeUint32(body, packet.senderID);
	writeUint32(body, packet.chatID);
	writeString(body, packet.text);
	return buildPacket(PacketType::MessageSend, messageID, sessionID, body);
}

std::vector<uint8_t> Serializer::buildDisconnectRequestPacket(uint32_t messageID, uint32_t sessionID)
{
	return buildPacket(PacketType::DisconnectRequest, messageID, sessionID);
}
