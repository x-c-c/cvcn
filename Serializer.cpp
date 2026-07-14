#include "Serializer.h"

void Serializer::serialize_uint16(const uint16_t field, std::vector<uint8_t>& buf, size_t& pos)
{
	buf[pos++] = static_cast<uint8_t>(field >> 8);
	buf[pos++] = static_cast<uint8_t>(field & 0xFF);
}

void Serializer::serialize_uint32(const uint32_t field, std::vector<uint8_t>& buf, size_t& pos)
{
	serialize_uint16(static_cast<uint16_t>(field >> 16), buf, pos);
	serialize_uint16(static_cast<uint16_t>(field & 0xFFFF), buf, pos);
}


std::vector<uint8_t> Serializer::serializeHeader(const PacketHeader& header)
{
	size_t bufLen = sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t);
	std::vector<uint8_t> buf(bufLen);
	size_t pos = 0;
	
	serialize_uint16(static_cast<uint16_t>(header.type_), buf, pos);
	serialize_uint32(header.messageID_, buf, pos);
	serialize_uint32(header.sessionID_, buf, pos);
	serialize_uint16(header.messageLen_, buf, pos);
	return buf; 
}


std::vector<uint8_t> Serializer::serializeBody(const PacketHeader& header)
{
	size_t bufLen = sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t);
	std::vector<uint8_t> buf(bufLen);
	size_t pos = 0;
	
	serialize_uint16(static_cast<uint16_t>(header.type_), buf, pos);
	serialize_uint32(header.messageID_, buf, pos);
	serialize_uint32(header.sessionID_, buf, pos);
	serialize_uint16(header.messageLen_, buf, pos);
	return buf; 
}
