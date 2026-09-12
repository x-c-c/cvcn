#pragma once
#include <cstdint>
#include <string>

enum class PacketType : uint16_t
{
    ConnectRequest      = 0x1,
    ConnectResponse     = 0x2,
    RegisterRequest     = 0x3,
    RegisterResponse    = 0x4,
    AuthRequest         = 0x5,
    AuthResponse        = 0x6,
    MessageSend         = 0x7,
    DisconnectRequest   = 0x8,
    DeleteRequest       = 0x9,
    DeleteResponse      = 0xA,
    FindUserRequest     = 0xB,
    FindUserResponse    = 0xC,
    CreateChatRequest   = 0xD,
    CreateChatResponse  = 0xE,
    ChatListRequest     = 0xF,
    ChatListResponse    = 0x10
};

#pragma pack(push, 1)
struct PacketHeaderRaw
{
    uint16_t type;          ///< Тип пакета (DataType).
    uint32_t messageID;     ///< Уникальный идентификатор сообщения.
    uint32_t sessionID;     ///< Идентификатор сессии.
    uint16_t messageLen;    ///< Длина тела пакета в байтах (после заголовка).
};
#pragma pack(pop)

struct ConnectRequestData {};
struct ConnectResponseData {};
struct RegisterRequestData
{
	std::string username;   ///< Имя пользователя.
	std::string password;   ///< Пароль (в будущем – хэш).
};
struct RegisterResponseData
{
	uint8_t success;        ///< 1 – успех, 0 – ошибка.
};
struct AuthRequestData
{
	std::string username;   ///< Имя пользователя.
	std::string password;   ///< Пароль (в будущем – хэш).
};
struct AuthResponseData
{
	uint8_t success;        ///< 1 – успех, 0 – ошибка.
};
struct MessageSendData
{
	uint32_t senderID;      ///< Идентификатор отправителя.
	uint32_t chatID;        ///< Идентификатор чата/получателя.
	std::string text;       ///< Текст сообщения.
};
struct DisconnectRequestData {};
struct DeleteRequestData
{
    std::string username;
    std::string password;
};
struct DeleteResponseData
{
    uint8_t success;
};
struct FindUserRequestData
{
    std::string query;
};
struct FindUserResponseData
{
    std::vector<std::string> usernames;
};

struct CreateChatRequestData
{
    std::string peerUsername;       ///< С кем создать 1:1 чат
};
struct CreateChatResponseData
{
    uint8_t  success;
    uint32_t chatID;
    std::string peerUsername;
};

struct ChatListEntry
{
    uint32_t chatID;
    std::string peerUsername;
};
struct ChatListRequestData {};
struct ChatListResponseData
{
    std::vector<ChatListEntry> chats;
};




