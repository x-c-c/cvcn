#pragma once
#include <cstdint>
#include <string>

namespace ByteReader
{
    uint16_t readUint16BE(const uint8_t*& cursor, size_t& remaining);
	uint32_t readUint32BE(const uint8_t*& cursor, size_t& remaining);
	std::string readString(const uint8_t*& cursor, size_t& remaining);
}

