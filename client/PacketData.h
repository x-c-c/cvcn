#pragma once
#include <cstdint>
#include <string>
#include <vector>

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
    ChatListResponse    = 0x10,
    MessageReceive      = 0x11
};

#pragma pack(push, 1)
struct PacketHeaderRaw
{
    uint16_t type;
    uint32_t messageID;
    uint32_t sessionID;
    uint16_t messageLen;
};
#pragma pack(pop)

struct ConnectRequestData {};
struct ConnectResponseData {};

struct RegisterRequestData
{
    std::string username;
    std::string password;
};
struct RegisterResponseData
{
    uint8_t success;
};

struct AuthRequestData
{
    std::string username;
    std::string password;
};
struct AuthResponseData
{
    uint8_t success;
};

struct MessageSendData
{
    uint32_t senderID;
    uint32_t chatID;
    std::string text;
};

struct MessageReceiveData
{
    uint32_t senderID;          ///< ID отправителя
    std::string senderUsername; ///< Имя отправителя, чтобы клиент не делал доп. запросов
    uint32_t chatID;
    std::string text;
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
    std::string peerUsername;
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
