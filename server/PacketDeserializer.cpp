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
    data.success = ByteReader::readUint8(cursor, remaining);
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
bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, MessageReceiveData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.senderID       = ByteReader::readUint32BE(cursor, remaining);
    data.senderUsername = ByteReader::readString(cursor, remaining);
    data.chatID         = ByteReader::readUint32BE(cursor, remaining);
    data.text           = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}
bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, DeleteRequestData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.username = ByteReader::readString(cursor, remaining);
    data.password = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, DeleteResponseData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.success = ByteReader::readUint8(cursor, remaining);
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, FindUserRequestData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.query = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, FindUserResponseData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    const uint16_t count = ByteReader::readUint16BE(cursor, remaining);
    data.usernames.clear();
    data.usernames.reserve(count);
    for (uint16_t i = 0; i < count; ++i)
        data.usernames.push_back(ByteReader::readString(cursor, remaining));
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, CreateChatRequestData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.peerUsername = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, CreateChatResponseData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.success      = ByteReader::readUint8(cursor, remaining);
    data.chatID       = ByteReader::readUint32BE(cursor, remaining);
    data.peerUsername = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, ChatListRequestData& data)
{
    (void)body;
    (void)data;
    return true;
}

bool PacketDeserializer::deserializeData(const std::vector<uint8_t>& body, ChatListResponseData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    const uint16_t count = ByteReader::readUint16BE(cursor, remaining);
    data.chats.clear();
    data.chats.reserve(count);
    for (uint16_t i = 0; i < count; ++i)
    {
        ChatListEntry entry;
        entry.chatID       = ByteReader::readUint32BE(cursor, remaining);
        entry.peerUsername = ByteReader::readString(cursor, remaining);
        data.chats.push_back(entry);
    }
    return remaining == 0;
}
