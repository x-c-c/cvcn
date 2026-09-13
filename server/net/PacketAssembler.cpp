#include "PacketAssembler.h"
#include <cstring>
#include <arpa/inet.h>

void PacketAssembler::appendData(const uint8_t* data, size_t len)
{
	size_t oldSize = readBuffer_.size();
	readBuffer_.resize(oldSize + len);
	std::memcpy(readBuffer_.data() + oldSize, data, len);
}

bool PacketAssembler::extractPacket(PacketHeaderRaw& header, std::vector<uint8_t>& body)
{
	if (readBuffer_.size() < sizeof(PacketHeaderRaw))
		return false;
	
	std::memcpy(&header, readBuffer_.data(), sizeof(header));
	header.type			= ntohs(header.type);
	header.messageID	= ntohl(header.messageID);
	header.sessionID	= ntohl(header.sessionID);
	header.messageLen	= ntohs(header.messageLen);

	size_t totalSize = sizeof(PacketHeaderRaw) + header.messageLen;
	if (readBuffer_.size() < totalSize)
		return false;

	body.assign(readBuffer_.begin() + sizeof(PacketHeaderRaw), readBuffer_.begin() + totalSize);
	readBuffer_.erase(readBuffer_.begin(), readBuffer_.begin() + totalSize);
	return true;
}
