#include "Serializer.h"
#include <cstring>
#include <arpa/inet.h>

/***************************************************/
/* === Запись === */
/***************************************************/
namespace
{
    void appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count)
    {
        const size_t offset = dest.size();
        dest.resize(offset + count);
        std::memcpy(dest.data() + offset, src, count);
    }
}

namespace BinaryWriter
{
    void writeUint16(std::vector<uint8_t>& buffer, uint16_t value)
    {
        const uint16_t networkOrder = htons(value);
        appendBytes(buffer, &networkOrder, sizeof(networkOrder));
    }
    void writeUint32(std::vector<uint8_t>& buffer, uint32_t value)
    {
        const uint32_t networkOrder = htonl(value);
        appendBytes(buffer, &networkOrder, sizeof(networkOrder));
    }
    void writeString(std::vector<uint8_t>& buffer, const std::string& str)
    {
        const uint16_t length = static_cast<uint16_t>(str.size());
        writeUint16(buffer, length);
        appendBytes(buffer, str.data(), length);
    }
}

    /***************************************************/
    /* === Чтение === */
    /***************************************************/
namespace BinaryReader
{
    uint16_t readUint16(const uint8_t*& cursor, size_t& remaining)
    {
        if (remaining < sizeof(uint16_t))
            return 0;
        uint16_t value;
        std::memcpy(&value, cursor, sizeof(value));
        value = ntohs(value);
        cursor += sizeof(uint16_t);
        remaining -= sizeof(uint16_t);
        return value;
    }
    uint32_t readUint32(const uint8_t*& cursor, size_t& remaining)
    {
        if (remaining < sizeof(uint32_t))
            return 0;
        uint32_t value;
        std::memcpy(&value, cursor, sizeof(value));
        value = ntohl(value);
        cursor += sizeof(uint32_t);
        remaining -= sizeof(uint32_t);
        return value;
    }

    std::string readString(const uint8_t*& cursor, size_t& remaining)
    {
        const uint16_t length = readUint16(cursor, remaining);
        if (remaining < length)
            return "";
        std::string str(reinterpret_cast<const char*>(cursor), length);
        cursor += length;
        remaining -= length;
        return str;
    }
}

/***************************************************/
/* === Десериализация заголовка === */
/***************************************************/

/**
 * @brief Извлекает и преобразует бинарный заголовок из сырых данных.
 * @param rawData байтовый вектор (не менее 12 байт)
 * @param header  результат (поля переведены в хост-порядок)
 * @return true, если заголовок успешно прочитан
 */
bool Serializer::deserializeHeader(const std::vector<uint8_t>& rawData, PacketHeaderRaw& header)
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

/***************************************************/
/* === Разбор тел === */
/***************************************************/

/**
 * @brief Разбирает пустое тело ConnectRequestPacket (всегда успешно).
 */
bool Serializer::parseConnectRequestPacket(const std::vector<uint8_t>& body, ConnectRequestPacket& packet)
{
	(void)body; (void)packet;
	return true;
}

/**
 * @brief Разбирает пустое тело ConnectResponsePacket (всегда успешно).
 */
bool Serializer::parseConnectResponsePacket(const std::vector<uint8_t>& body, ConnectResponsePacket& packet)
{
	(void)body; (void)packet;
	return true;
}

/**
 * @brief Разбирает тело RegisterRequestPacket (две строки).
 */
bool Serializer::parseRegisterRequestPacket(const std::vector<uint8_t>& body, RegisterRequestPacket& packet)
{
	const uint8_t* cursor = body.data();
	size_t remaining = body.size();
	packet.username = readString(cursor, remaining);
	packet.password = readString(cursor, remaining);
	return remaining == 0;
}

/**
 * @brief Разбирает тело RegisterResponsePacket (один байт успеха).
 */
bool Serializer::parseRegisterResponsePacket(const std::vector<uint8_t>& body, RegisterResponsePacket& packet)
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
 * @brief Разбирает тело AuthRequestPacket (две строки).
 */
bool Serializer::parseAuthRequestPacket(const std::vector<uint8_t>& body, AuthRequestPacket& packet)
{
	const uint8_t* cursor = body.data();
	size_t remaining = body.size();
	packet.username = readString(cursor, remaining);
	packet.password = readString(cursor, remaining);
	return remaining == 0;
}

/**
 * @brief Разбирает тело AuthResponsePacket (один байт успеха).
 */
bool Serializer::parseAuthResponsePacket(const std::vector<uint8_t>& body, AuthResponsePacket& packet)
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
bool Serializer::parseMessageSendPacket(const std::vector<uint8_t>& body, MessageSendPacket& packet)
{
	const uint8_t* cursor = body.data();
	size_t remaining = body.size();
	packet.senderID = readUint32(cursor, remaining);
	packet.chatID   = readUint32(cursor, remaining);
	packet.text     = readString(cursor, remaining);
	return remaining == 0;
}

/**
 * @brief Разбирает пустое тело DisconnectRequestPacket (всегда успешно).
 */
bool Serializer::parseDisconnectRequestPacket(const std::vector<uint8_t>& body, DisconnectRequestPacket& packet)
{
	(void)body; (void)packet;
	return true;
}
