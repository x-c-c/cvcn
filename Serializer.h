#pragma once
#include "Packets.h"
#include <cstdint>
#include <vector>

class Serializer
{
public:
	// Конкретная сборка пакета (сериализация)
	static std::vector<uint8_t> buildConnectRequestPacket(uint32_t messageID, uint32_t sessionID);
	static std::vector<uint8_t> buildConnectResponsePacket(uint32_t messageID, uint32_t sessionID);
	static std::vector<uint8_t> buildRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet);
	static std::vector<uint8_t> buildRegisterResponsePacket(uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket& packet);
	static std::vector<uint8_t> buildAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet);
	static std::vector<uint8_t> buildAuthResponsePacket(uint32_t messageID, uint32_t sessionID, const AuthResponsePacket& packet);
	static std::vector<uint8_t> buildMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet);
	static std::vector<uint8_t> buildDisconnectRequestPacket(uint32_t messageID, uint32_t sessionID);
	
	// Десериализация потока байт в заголовок
	static bool deserializeHeader(const std::vector<uint8_t>& data, PacketHeaderRaw& header);
	
	// Десериализация потока байт в объект
	static bool parseConnectRequestPacket(const std::vector<uint8_t>& body, ConnectRequestPacket& packet);
	static bool parseConnectResponsePacket(const std::vector<uint8_t>& body, ConnectResponsePacket& packet);
	static bool parseRegisterRequestPacket(const std::vector<uint8_t>& body, RegisterRequestPacket& packet);
	static bool parseRegisterResponsePacket(const std::vector<uint8_t>& body, RegisterResponsePacket& packet);
	static bool parseAuthRequestPacket(const std::vector<uint8_t>& body, AuthRequestPacket& packet);
	static bool parseAuthResponsePacket(const std::vector<uint8_t>& body, AuthResponsePacket& packet);
	static bool parseMessageSendPacket(const std::vector<uint8_t>& body, MessageSendPacket& packet);
	static bool parseDisconnectRequestPacket(const std::vector<uint8_t>& body, DisconnectRequestPacket& packet);
private:
	// Запись байт в вектор
	static void appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count);
	static void writeUint16(std::vector<uint8_t>& buf, uint16_t val);
	static void writeUint32(std::vector<uint8_t>& buf, uint32_t val);
	static void writeString(std::vector<uint8_t>& buf, const std::string& str);
	
	// Чтение байт из вектора
	static uint16_t readUint16(const uint8_t*& data, size_t& remaining);
	static uint32_t readUint32(const uint8_t*& data, size_t& remaining);
	static std::string readString(const uint8_t*& data, size_t& remaining);
	
	// Универсальная сборка пакета (сериализация)
	static std::vector<uint8_t> buildPacket(PacketType type, uint32_t messageID, uint32_t sessionID, const std::vector<uint8_t>& body = {});
};
