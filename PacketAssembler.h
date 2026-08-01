#pragma once
#include "Packets.h"
#include <vector>
#include <cstdint>

class PacketAssembler
{
public:
	void appendData(const uint8_t* data, size_t len);
	bool extractPacket(PacketHeaderRaw& header, std::vector<uint8_t>& body);
private:
	std::vector<uint8_t> readBuffer_;
	static constexpr size_t MAX_BUFFER_SIZE = 64 * 1024;
};
