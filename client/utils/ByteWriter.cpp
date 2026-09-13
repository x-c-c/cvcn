#include "ByteWriter.h"
#include <cstring>
#include <arpa/inet.h>

namespace ByteWriter
{
    void appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count)         // TODO: пересмотреть способ записи значений в вектор
	{
		const size_t offset = dest.size();
		dest.resize(offset + count);
		std::memcpy(dest.data() + offset, src, count);
	}
        void writeUint8(std::vector<uint8_t>& buffer, uint8_t value)
        {
         appendBytes(buffer, &value, sizeof(value));
        }
	void writeUint16BE(std::vector<uint8_t>& buffer, uint16_t value)
	{
		const uint16_t networkOrder = htons(value);
		appendBytes(buffer, &networkOrder, sizeof(networkOrder));
	}
	void writeUint32BE(std::vector<uint8_t>& buffer, uint32_t value)
	{
		const uint32_t networkOrder = htonl(value);
		appendBytes(buffer, &networkOrder, sizeof(networkOrder));
	}
		
	void writeString(std::vector<uint8_t>& buffer, const std::string& str)
	{
		const uint16_t length = static_cast<uint16_t>(str.size());
		writeUint16BE(buffer, length);
		appendBytes(buffer, str.data(), length);
	}
}
