/**
 * @file Serializer.cpp
 * @brief Реализация сериализации/десериализации бинарного протокола.
 */

#include "Serializer.h"
#include <cstring>
#include <arpa/inet.h>

/***************************************************/
/* === Запись === */
/***************************************************/

/**
 * @brief Дописывает count байт из src в конец вектора dest.
 * @param dest целевой вектор
 * @param src  указатель на исходные данные
 * @param count количество байт
 */
void Serializer::appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count)
{
	const size_t offset = dest.size();
	dest.resize(offset + count);
	std::memcpy(dest.data() + offset, src, count);
}

/**
 * @brief Записывает 16-битное число в сетевом порядке (big-endian).
 * @param buffer целевой буфер
 * @param value  значение
 */
void Serializer::writeUint16(std::vector<uint8_t>& buffer, uint16_t value)
{
	const uint16_t networkOrder = htons(value);
	appendBytes(buffer, &networkOrder, sizeof(networkOrder));
}

/**
 * @brief Записывает 32-битное число в сетевом порядке.
 * @param buffer целевой буфер
 * @param value  значение
 */
void Serializer::writeUint32(std::vector<uint8_t>& buffer, uint32_t value)
{
	const uint32_t networkOrder = htonl(value);
	appendBytes(buffer, &networkOrder, sizeof(networkOrder));
}

/**
 * @brief Записывает строку в формате: [длина 2 байта][данные].
 * @param buffer целевой буфер
 * @param str    строка
 */
void Serializer::writeString(std::vector<uint8_t>& buffer, const std::string& str)
{
	const uint16_t length = static_cast<uint16_t>(str.size());
	writeUint16(buffer, length);
	appendBytes(buffer, str.data(), length);
}

/***************************************************/
/* === Чтение === */
/***************************************************/

/**
 * @brief Читает 16-битное число из сетевого порядка, сдвигает курсор.
 * @param cursor    ссылка на указатель чтения, будет продвинут вперёд
 * @param remaining количество оставшихся байт, уменьшится
 * @return прочитанное значение (0, если данных недостаточно)
 */
uint16_t Serializer::readUint16(const uint8_t*& cursor, size_t& remaining)
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

/**
 * @brief Читает 32-битное число из сетевого порядка, сдвигает курсор.
 * @param cursor    ссылка на указатель чтения
 * @param remaining оставшиеся байты
 * @return прочитанное значение
 */
uint32_t Serializer::readUint32(const uint8_t*& cursor, size_t& remaining)
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

/**
 * @brief Читает строку, предварённую двухбайтовой длиной.
 * @param cursor    ссылка на указатель чтения
 * @param remaining оставшиеся байты
 * @return прочитанная строка (пустая, если данных недостаточно)
 */
std::string Serializer::readString(const uint8_t*& cursor, size_t& remaining)
{
	const uint16_t length = readUint16(cursor, remaining);
	if (remaining < length)
		return "";
	std::string str(reinterpret_cast<const char*>(cursor), length);
	cursor += length;
	remaining -= length;
	return str;
}

/***************************************************/
/* === Универсальная сборка === */
/***************************************************/

/**
 * @brief Собирает полный пакет из заголовка и тела.
 * @param type тип пакета
 * @param messageID идентификатор сообщения
 * @param sessionID идентификатор сессии
 * @param body тело пакета (может быть пустым)
 * @return байтовый вектор готового пакета
 */
std::vector<uint8_t> Serializer::buildPacket(PacketType type, uint32_t messageID, uint32_t sessionID, const std::vector<uint8_t>& body)
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

/***************************************************/
/* === Конкретные пакеты === */
/***************************************************/

/**
 * @brief Создаёт пакет ConnectRequest (без тела).
 */
std::vector<uint8_t> Serializer::buildConnectRequestPacket(uint32_t messageID, uint32_t sessionID)
{
	return buildPacket(PacketType::ConnectRequest, messageID, sessionID);
}

/**
 * @brief Создаёт пакет ConnectResponse (без тела).
 */
std::vector<uint8_t> Serializer::buildConnectResponsePacket(uint32_t messageID, uint32_t sessionID)
{
	return buildPacket(PacketType::ConnectResponse, messageID, sessionID);
}

/**
 * @brief Создаёт пакет RegisterRequest с учётными данными.
 */
std::vector<uint8_t> Serializer::buildRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet)
{
	std::vector<uint8_t> body;
	writeString(body, packet.username);
	writeString(body, packet.password);
	return buildPacket(PacketType::RegisterRequest, messageID, sessionID, body);
}

/**
 * @brief Создаёт пакет RegisterResponse с результатом.
 */
std::vector<uint8_t> Serializer::buildRegisterResponsePacket(uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket& packet)
{
	std::vector<uint8_t> body;
	body.push_back(packet.success);
	return buildPacket(PacketType::RegisterResponse, messageID, sessionID, body);
}

/**
 * @brief Создаёт пакет AuthRequest с учётными данными.
 */
std::vector<uint8_t> Serializer::buildAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet)
{
	std::vector<uint8_t> body;
	writeString(body, packet.username);
	writeString(body, packet.password);
	return buildPacket(PacketType::AuthRequest, messageID, sessionID, body);
}

/**
 * @brief Создаёт пакет AuthResponse с результатом.
 */
std::vector<uint8_t> Serializer::buildAuthResponsePacket(uint32_t messageID, uint32_t sessionID, const AuthResponsePacket& packet)
{
	std::vector<uint8_t> body;
	body.push_back(packet.success);
	return buildPacket(PacketType::AuthResponse, messageID, sessionID, body);
}

/**
 * @brief Создаёт пакет MessageSend.
 */
std::vector<uint8_t> Serializer::buildMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet)
{
	std::vector<uint8_t> body;
	writeUint32(body, packet.senderID);
	writeUint32(body, packet.chatID);
	writeString(body, packet.text);
	return buildPacket(PacketType::MessageSend, messageID, sessionID, body);
}

/**
 * @brief Создаёт пакет DisconnectRequest (без тела).
 */
std::vector<uint8_t> Serializer::buildDisconnectRequestPacket(uint32_t messageID, uint32_t sessionID)
{
	return buildPacket(PacketType::DisconnectRequest, messageID, sessionID);
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
