#pragma once
#include <cstdint>
#include <string>

enum class PacketType : uint16_t
{
	ConnectRequest	= 0x1,
	ConnectResponse,

	RegisterRequest,
	RegisterResponse,

	AuthRequest,
	AuthResoponse,

	MessageSend,
	DisconnectRequest
};


struct ParentPacket
{
public:
	PacketType type_;
	uint32_t messageID_;
	uint32_t sessionID_;
	uint16_t messageLen_;
};

struct ConnectRequestPacket:	public ParentPacket{	ConnectRequestPacket()	{ type_ = PacketType::ConnectRequest;	}	};
struct ConnectResponsePacket:	public ParentPacket{	ConnectResponsePacket()	{ type_ = PacketType::ConnectResponse;	}	};

struct RegisterRequestPacket:	public ParentPacket
{
public:
	std::string username;
	std::string password;	// change later to hash or smth

	RegisterRequestPacket(){ type_ = PacketType::RegisterRequest; }
};
struct RegisterResoponsePacket:	public ParentPacket{	RegisterResponsePacket(){ type_ = PacketType::RegisterResponse;	}	};

struct AuthRequestPacket:	public ParentPacket
{
public:
	std::string username;
	std::string password;	// change later to hash or smth

	AuthRequestPacket(){ type_ = PacketType::AuthRequest; }
};
struct AuthResoponsePacket:	public ParentPacket{	AuthResponsePacket()	{ type_ = PacketType::AuthResponse;	}	};

struct MessageSend:	Public ParentPacket
{
public:
	
};


