#pragma once
#include "PacketData.h"
#include <cstdint>
#include <vector>

class PacketDeserializer
{
public:
    static bool deserializeHeader(const std::vector<uint8_t>& rawData, PacketHeaderRaw& header);

    static bool deserializeData(const std::vector<uint8_t>& body, ConnectRequestData& data);
    static bool deserializeData(const std::vector<uint8_t>& body, ConnectResponseData& data);

    static bool deserializeData(const std::vector<uint8_t>& body, RegisterRequestData& data);
    static bool deserializeData(const std::vector<uint8_t>& body, RegisterResponseData& data);

    static bool deserializeData(const std::vector<uint8_t>& body, AuthRequestData& data);
    static bool deserializeData(const std::vector<uint8_t>& body, AuthResponseData& data);

    static bool deserializeData(const std::vector<uint8_t>& body, MessageSendData& data);

    static bool deserializeData(const std::vector<uint8_t>& body, DisconnectRequestData& data);

    static bool deserializeData(const std::vector<uint8_t>& body, DeleteRequestData& data);
    static bool deserializeData(const std::vector<uint8_t>& body, DeleteResponseData& data);

    static bool deserializeData(const std::vector<uint8_t>& body, FindUserRequestData& data);
    static bool deserializeData(const std::vector<uint8_t>& body, FindUserResponseData& data);

    static bool deserializeData(const std::vector<uint8_t>& body, CreateChatRequestData& data);
    static bool deserializeData(const std::vector<uint8_t>& body, CreateChatResponseData& data);

    static bool deserializeData(const std::vector<uint8_t>& body, ChatListRequestData& data);
    static bool deserializeData(const std::vector<uint8_t>& body, ChatListResponseData& data);
};
