#include "PacketParser.h"
#include "ByteReader.h"
#include <cstring>
#include <arpa/inet.h>

bool PacketParser::deserializeHeader(const std::vector<uint8_t>& rawData, PacketHeaderRaw& header)
{
	if (rawData.size() < sizeof(header))
		return false;

	std::memcpy(&header, rawData.data(), sizeof(header));
	header.type      = ntohs(header.type);
	header.messageID = ntohl(header.messageID);
	header.sessionID = ntohl(header.sessionID);
	header.messageLen = ntohs(header.messageLen);
	return true;
}
bool PacketParser::parseConnectRequestPacket(const std::vector<uint8_t>& body, ConnectRequestPacket& packet)
{
    (void)body;
    (void)packet;
    return true;
}
bool PacketParser::parseConnectResponsePacket(const std::vector<uint8_t>& body, ConnectResponsePacket& packet)
{
    (void)body;
    (void)packet;
    return true;
}
bool PacketParser::parseRegisterRequestPacket(const std::vector<uint8_t>& body, RegisterRequestPacket& packet)
{
	const uint8_t* cursor = body.data();
	size_t remaining = body.size();
    packet.username = ByteReader::readString(cursor, remaining);
    packet.password = ByteReader::readString(cursor, remaining);
	return remaining == 0;
}
bool PacketParser::parseRegisterResponsePacket(const std::vector<uint8_t>& body, RegisterResponsePacket& packet)
{
	size_t remaining = body.size();
	if (remaining < sizeof(uint8_t))
		return false;
	const uint8_t* cursor = body.data();
	packet.success = *cursor;
	cursor += sizeof(uint8_t);
	remaining -= sizeof(uint8_t);
	return remaining == 0;
}
bool PacketParser::parseAuthRequestPacket(const std::vector<uint8_t>& body, AuthRequestPacket& packet)
{
	const uint8_t* cursor = body.data();
	size_t remaining = body.size();
    packet.username = ByteReader::readString(cursor, remaining);
    packet.password = ByteReader::readString(cursor, remaining);
	return remaining == 0;
}

/**
 * @brief Разбирает тело AuthResponsePacket (один байт успеха).
 */
bool PacketParser::parseAuthResponsePacket(const std::vector<uint8_t>& body, AuthResponsePacket& packet)
{
	size_t remaining = body.size();
	if (remaining < sizeof(uint8_t))
		return false;
	const uint8_t* cursor = body.data();
	packet.success = *cursor;
	cursor += sizeof(uint8_t);
	remaining -= sizeof(uint8_t);
	return remaining == 0;
}

/**
 * @brief Разбирает тело MessageSendPacket (senderID, chatID, text).
 */
bool PacketParser::parseMessageSendPacket(const std::vector<uint8_t>& body, MessageSendPacket& packet)
{
	const uint8_t* cursor = body.data();
	size_t remaining = body.size();
	packet.senderID = readUint32(cursor, remaining);
	packet.chatID   = readUint32(cursor, remaining);
    packet.text     = ByteReader::readString(cursor, remaining);
	return remaining == 0;
}

/**
 * @brief Разбирает пустое тело DisconnectRequestPacket (всегда успешно).
 */
bool PacketParser::parseDisconnectRequestPacket(const std::vector<uint8_t>& body, DisconnectRequestPacket& packet)
{
	(void)body; (void)packet;
	return true;
}
