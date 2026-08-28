#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H
#include <vector>
#include <unistd.h>
#include "Packets.h"
class PacketBuilder
{
public:
    PacketBuilder();
    std::vector<uint8_t> buildPacket(Packet::);


};

#endif // PACKETBUILDER_H
