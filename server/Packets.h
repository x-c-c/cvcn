/**
 * @file Packets.h
 * @brief Типы пакетов и структуры данных, используемые в бинарном протоколе.
 */

#pragma once
#include <cstdint>
#include <string>

/**
 * @brief Типы пакетов протокола.
 */
enum class PacketType : uint16_t
{
	ConnectRequest      = 0x1,  ///< Запрос подключения (без тела).
	ConnectResponse     = 0x2,  ///< Ответ на подключение (без тела).
	RegisterRequest     = 0x3,  ///< Запрос регистрации.
	RegisterResponse    = 0x4,  ///< Ответ на регистрацию.
	AuthRequest         = 0x5,  ///< Запрос аутентификации.
	AuthResponse        = 0x6,  ///< Ответ на аутентификацию.
	MessageSend         = 0x7,  ///< Отправка текстового сообщения.
	DisconnectRequest   = 0x8   ///< Запрос отключения (без тела).
};

#pragma pack(push, 1)
/**
 * @brief Бинарный заголовок пакета, передаваемый по сети (big‑endian).
 *
 * Используется исключительно внутри Serializer для сериализации/десериализации.
 */
struct PacketHeaderRaw
{
	uint16_t type;          ///< Тип пакета (PacketType).
	uint32_t messageID;     ///< Уникальный идентификатор сообщения.
	uint32_t sessionID;     ///< Идентификатор сессии.
	uint16_t messageLen;    ///< Длина тела пакета в байтах (после заголовка).
};
#pragma pack(pop)

/** @brief Запрос подключения (тело отсутствует). */
struct ConnectRequestPacket {};

/** @brief Ответ на подключение (тело отсутствует). */
struct ConnectResponsePacket {};

/**
 * @brief Запрос регистрации.
 */
struct RegisterRequestPacket
{
	std::string username;   ///< Имя пользователя.
	std::string password;   ///< Пароль (в будущем – хэш).
};

/**
 * @brief Ответ на регистрацию.
 */
struct RegisterResponsePacket
{
	uint8_t success;        ///< 1 – успех, 0 – ошибка.
};

/**
 * @brief Запрос аутентификации.
 */
struct AuthRequestPacket
{
	std::string username;   ///< Имя пользователя.
	std::string password;   ///< Пароль (в будущем – хэш).
};

/**
 * @brief Ответ на аутентификацию.
 */
struct AuthResponsePacket
{
	uint8_t success;        ///< 1 – успех, 0 – ошибка.
};

/**
 * @brief Отправка текстового сообщения.
 */
struct MessageSendPacket
{
	uint32_t senderID;      ///< Идентификатор отправителя.
	uint32_t chatID;        ///< Идентификатор чата/получателя.
	std::string text;       ///< Текст сообщения.
};

/** @brief Запрос отключения (тело отсутствует). */
struct DisconnectRequestPacket {};
