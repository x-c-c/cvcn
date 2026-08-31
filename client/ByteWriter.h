#pragma once
#include <vector>
#include <cstdint>
#include <string>

namespace ByteWriter
{
    void appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count);
    void writeUint16BE(std::vector<uint8_t>& buffer, uint16_t value);
    void writeUint32BE(std::vector<uint8_t>& buffer, uint32_t value);
    void writeString(std::vector<uint8_t>& buffer, const std::string& str);
}
