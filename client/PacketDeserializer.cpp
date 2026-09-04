#include "PacketDeserializer.h"
#include "ByteReader.h"
#include <cstring>
#include <arpa/inet.h>

bool PacketDeserializer::deserializeHeader(const std::vector<uint8_t>& rawData, PacketHeaderRaw& header)
{
    if (rawData.size() < sizeof(header))
        return false;

    std::memcpy(&header, rawData.data(), sizeof(header));
    header.type      = ntohs(header.type);
    header.messageID = ntohl(header.messageID);
    header.sessionID = ntohl(header.sessionID);
    header.messageLen = ntohs(header.messageLen);
    return true;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, ConnectRequestData& data)
{
    (void)body;
    (void)data;
    return true;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, ConnectResponseData& data)
{
    (void)body;
    (void)data;
    return true;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, RegisterRequestData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.username = ByteReader::readString(cursor, remaining);
    data.password = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, RegisterResponseData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.success = ByteReader::readUint8(cursor, remaining);
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, AuthRequestData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.username = ByteReader::readString(cursor, remaining);
    data.password = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, AuthResponseData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.username = ByteReader::readString(cursor, remaining);
    data.password = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}
bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, MessageSendData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.senderID = ByteReader::readUint32BE(cursor, remaining);
    data.chatID   = ByteReader::readUint32BE(cursor, remaining);
    data.text     = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, DisconnectRequestData& data)
{
    (void)body;
    (void)data;
    return true;
}
