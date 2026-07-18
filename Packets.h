#pragma once
#include <cstdint>
#include <string>

enum class PacketType : uint16_t
{
	ConnectRequest		= 0x1,
	ConnectResponse		= 0x2,
	RegisterRequest		= 0x3,
	RegisterResponse	= 0x4,
	AuthRequest			= 0x5,
	AuthResponse		= 0x6,
	MessageSend			= 0x7,
	DisconnectRequest	= 0x8
};

#pragma pack(push, 1)
// бинарный заголовок, плотно упакованный, используется только внутри Serializer
struct PacketHeaderRaw
{
	uint16_t type_;
	uint32_t messageID_		= 0;
	uint32_t sessionID_		= 0;
	uint16_t messageLen_	= 0;
};
#pragma pack(pop)

struct ConnectRequestPacket
{
	// без тела
};
struct ConnectResponsePacket
{
	// без тела
};

struct RegisterRequestPacket
{
	std::string username;
	std::string password;	// изменить позже на ХЭШ
};
struct RegisterResponsePacket
{
	uint8_t success;	// 1 - true, 0 - false
};

struct AuthRequestPacket
{
	std::string username;
	std::string password;	// изменить позже на ХЭШ
};
struct AuthResponsePacket
{
	uint8_t success;	// 1 - true, 0 - false
};

struct MessageSendPacket
{
	uint32_t senderID;
	uint32_t chatID;
	std::string text;
};

struct DisconnectRequestPacket
{
	// без тела
};


