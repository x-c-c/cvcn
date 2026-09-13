#include "ByteReader.h"
#include <cstring>
#include <arpa/inet.h>

namespace ByteReader
{
    uint8_t readUint8(const uint8_t*& cursor, size_t& remaining)
    {
        if (remaining < sizeof(uint8_t))
            return 0;
        uint8_t value = 0;
        std::memcpy(&value, cursor, sizeof(value));
        cursor += sizeof(uint8_t);
        remaining -= sizeof(uint8_t);
        return value;
    }

	uint16_t readUint16BE(const uint8_t*& cursor, size_t& remaining)
    {
        if (remaining < sizeof(uint16_t))
            return 0;
        uint16_t value = 0;
        std::memcpy(&value, cursor, sizeof(value));
        value = ntohs(value);
        cursor += sizeof(uint16_t);
        remaining -= sizeof(uint16_t);
        return value;
    }
    uint32_t readUint32BE(const uint8_t*& cursor, size_t& remaining)
    {
        if (remaining < sizeof(uint32_t))
            return 0;
        uint32_t value = 0;
        std::memcpy(&value, cursor, sizeof(value));
        value = ntohl(value);
        cursor += sizeof(uint32_t);
        remaining -= sizeof(uint32_t);
        return value;
    }

    std::string readString(const uint8_t*& cursor, size_t& remaining)
    {
        const uint16_t length = readUint16BE(cursor, remaining);
        if (remaining < length)
            return "";
        std::string str(reinterpret_cast<const char*>(cursor), length);
        cursor += length;
        remaining -= length;
        return str;
    }
}

