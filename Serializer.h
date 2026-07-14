#pragma once
#include "Packets.h"
#include <cstdint>
#include <vector>
class Serializer
{
public:
	void serialize_uint16(const uint16_t field, std::vector<uint8_t>& buf, size_t& pos);
	void serialize_uint32(const uint32_t field, std::vector<uint8_t>& buf, size_t& pos);
	std::vector<uint8_t> serializeHeader(const PacketHeader& header);
	std::vector<uint8_t> serializeBody(const PacketHeader& header);
};
