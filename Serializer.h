#pragma once
#include "Packets.h"
#include <cstdint>
#include <vector>

class Serializer
{
public:
	// сборка полного пакета [заголовок] + [тело]
	static std::vector<uint8_t> buildConnectRequestPacket(		uint32_t messageID, uint32_t sessionID);
	static std::vector<uint8_t> buildConnectResponsePacket(		uint32_t messageID, uint32_t sessionID);
	static std::vector<uint8_t> buildRegisterRequestPacket(		uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet);
	static std::vector<uint8_t> buildRegisterResponsePacket(	uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket& packet);
	static std::vector<uint8_t> buildAuthRequestPacket(			uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet);
	static std::vector<uint8_t> buildAuthResponsePacket(		uint32_t messageID, uint32_t sessionID, const AuthResponsePacket& packet);
	static std::vector<uint8_t> buildMessageSendPacket(			uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet);
	static std::vector<uint8_t> buildDisconnectRequestPacket(	uint32_t messageID, uint32_t sessionID);
	
private:
	static void appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count);
	static void writeUint16(std::vector<uint8_t>& buf, uint16_t val);
	static void writeUint32(std::vector<uint8_t>& buf, uint32_t val);
	static void writeString(std::vector<uint8_t>& buf, const std::string& str);
	static std::vector<uint8_t> buildPacket(PacketType type, uint32_t messageID, uint32_t sessionID, const std::vector<uint8_t>& body = {});
};
