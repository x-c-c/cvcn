#include "Serializer.h"
#include <cstring>
#include <arpa/inet.h>
/***************************************************/
/* === Запись байт объекта в вектор === */
/***************************************************/
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

/***************************************************/
/* === Чтение байт из вектора в объект === */
/***************************************************/
uint16_t Serializer::readUint16(const uint8_t*& data, size_t& remaining)
{
	if (remaining < sizeof(uint16_t))
		return 0;
	uint16_t val;
	std::memcpy(&val, data, sizeof(uint16_t));
	val = ntohs(val);
	data += sizeof(uint16_t);
	remaining -= sizeof(uint16_t);
	return val;
}
uint32_t Serializer::readUint32(const uint8_t*& data, size_t& remaining)
{
	if (remaining < sizeof(uint32_t))
		return 0;
	uint32_t val;
	std::memcpy(&val, data, sizeof(uint32_t));
	val = ntohl(val);
	data += sizeof(uint32_t);
	remaining -= sizeof(uint32_t);
	return val;
}
std::string Serializer::readString(const uint8_t*& data, size_t& remaining)
{
	const uint16_t len = readUint16(data, remaining);
	if (remaining < len)
		return "";
	std::string str(reinterpret_cast<const char*>(data), len);
	data += len;
	remaining -= len;
	return str;
}

/***************************************************/
/* === Универсальная сборка пакета (сериализация) === */
/***************************************************/
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
/***************************************************/
/* === Конкретная сборка пакета (сериализация) === */
/***************************************************/
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

/***************************************************/
/* ===  Десериализация потока байт в заголовок */
/***************************************************/
bool Serializer::deserializeHeader(const std::vector<uint8_t>& data, PacketHeaderRaw& header)
{
	if (data.size() < sizeof(header))
	{
		return false;
	}
	std::memcpy(&header, data.data(), sizeof(header));
	header.type_		= ntohs(header.type_);
	header.messageID_	= ntohl(header.messageID_);
	header.sessionID_	= ntohl(header.sessionID_);
	header.messageLen_	= ntohs(header.messageLen_);
	return true;
}

bool Serializer::parseConnectRequestPacket(const std::vector<uint8_t>& body, ConnectRequestPacket& packet)
{
	// Пустое тело
	(void)body;
	(void)packet;
	return true;
}
bool Serializer::parseConnectResponsePacket(const std::vector<uint8_t>& body, ConnectResponsePacket& packet)
{
	// Пустое тело
	(void)body;
	(void)packet;
	return true;
}
bool Serializer::parseRegisterRequestPacket(const std::vector<uint8_t>& body, RegisterRequestPacket& packet)
{
	const uint8_t* data = body.data();
	size_t remaining = body.size();
	packet.username = readString(data, remaining);
	packet.password = readString(data, remaining);
	return remaining == 0;
}
bool Serializer::parseRegisterResponsePacket(const std::vector<uint8_t>& body, RegisterResponsePacket& packet)
{
	size_t remaining = body.size();
	if (remaining < sizeof(uint8_t))
		return false;
	const uint8_t* data = body.data();
	packet.success = *data;
	data += sizeof(uint8_t);
	remaining -= sizeof(uint8_t);
	return remaining == 0;
}
bool Serializer::parseAuthRequestPacket(const std::vector<uint8_t>& body, AuthRequestPacket& packet)
{
	const uint8_t* data = body.data();
	size_t remaining = body.size();
	packet.username = readString(data, remaining);
	packet.password = readString(data, remaining);
	return remaining == 0;
}
bool Serializer::parseAuthResponsePacket(const std::vector<uint8_t>& body, AuthResponsePacket& packet)
{
	size_t remaining = body.size();
	if (remaining < sizeof(uint8_t))
		return false;
	const uint8_t* data = body.data();
	packet.success = *data;
	data += sizeof(uint8_t);
	remaining -= sizeof(uint8_t);
	return remaining == 0;
}
bool Serializer::parseMessageSendPacket(const std::vector<uint8_t>& body, MessageSendPacket& packet)
{
	const uint8_t* data = body.data();
	size_t remaining = body.size();
	packet.senderID = readUint32(data, remaining);
	packet.chatID   = readUint32(data, remaining);
	packet.text     = readString(data, remaining);
	return remaining == 0;
}
bool Serializer::parseDisconnectRequestPacket(const std::vector<uint8_t>& body, DisconnectRequestPacket& packet)
{
	(void)body;
	(void)packet;
	return true;
}
