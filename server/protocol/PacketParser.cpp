/**
 * @file    PacketParser.cpp
 * @brief   Реализация PacketParser.
 *
 * @details
 *   Разбор идёт курсором по body: ByteReader::read* двигает указатель вперёд
 *   и уменьшает remaining. В конце каждой перегрузки проверяется
 *   remaining == 0, что гарантирует: тело разобрано ровно, без
 *   хвостов и недобора.
 *
 *   Если ByteReader наткнётся на нехватку байт, он вернёт 0 / ""
 *   и оставит remaining как есть. Проверка remaining == 0 в конце
 *   поймает это и вернёт false — так парсер не пропустит битый пакет.
 *
 * @see PacketParser.h
 */

#include "./PacketParser.h"
#include "./ByteReader.h"
#include <cstring>
#include <arpa/inet.h>

bool PacketParser::deserializeHeader(const std::vector<uint8_t>& rawData,
                                     PacketHeaderRaw& header)
{
    // Заголовок фиксированного размера: без 12 байт разбирать нечего.
    if (rawData.size() < sizeof(header))
        return false;

    // Копируем 12 байт как есть — структура упакована, поэтому
    // memcpy в неё безопасен (нет padding-а).
    std::memcpy(&header, rawData.data(), sizeof(header));

    // network → host. type и messageLen — 2 байта, остальные — 4.
    header.type       = ntohs(header.type);
    header.messageID  = ntohl(header.messageID);
    header.sessionID  = ntohl(header.sessionID);
    header.messageLen = ntohs(header.messageLen);
    return true;
}

// Пустые тела 
// ConnectRequest/Response и DisconnectRequest не несут данных.
// Проверять нечего: если тело и было, оно уже отброшено assembler-ом
// (extractPacket режет строго по messageLen).
bool PacketParser::deserializeData(const std::vector<uint8_t>& body,
                                   ConnectRequestData& data)
{
    (void)body;
    (void)data;
    return true;
}

bool PacketParser::deserializeData(const std::vector<uint8_t>& body,
                                   ConnectResponseData& data)
{
    (void)body;
    (void)data;
    return true;
}

//  RegisterRequest: две строки с префиксом длины 
bool PacketParser::deserializeData(const std::vector<uint8_t>& body,
                                   RegisterRequestData& data)
{
    // Курсор и remaining инициализируются от body. ByteReader
    // продвигает оба по мере чтения.
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();

    data.username = ByteReader::readString(cursor, remaining);
    data.password = ByteReader::readString(cursor, remaining);

    // remaining == 0 — прочитано ровно столько, сколько было.
    // Иначе в теле есть хвост или ByteReader упёрся в границу.
    return remaining == 0;
}

//  RegisterResponse: один байт success 
bool PacketParser::deserializeData(const std::vector<uint8_t>& body,
                                   RegisterResponseData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.success = ByteReader::readUint8(cursor, remaining);
    return remaining == 0;
}

//  AuthRequest: две строки 
bool PacketParser::deserializeData(const std::vector<uint8_t>& body,
                                   AuthRequestData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.username = ByteReader::readString(cursor, remaining);
    data.password = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}

//  AuthResponse: один байт success 
bool PacketParser::deserializeData(const std::vector<uint8_t>& body,
                                   AuthResponseData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.success = ByteReader::readUint8(cursor, remaining);
    return remaining == 0;
}

//  MessageSend: два uint32 + строка 
bool PacketParser::deserializeData(const std::vector<uint8_t>& body,
                                   MessageSendData& data)
{
    const uint8_t* cursor = body.data();
    size_t remaining = body.size();
    data.senderID = ByteReader::readUint32BE(cursor, remaining);
    data.chatID   = ByteReader::readUint32BE(cursor, remaining);
    data.text     = ByteReader::readString(cursor, remaining);
    return remaining == 0;
}

//  DisconnectRequest: тело пустое 
bool PacketParser::deserializeData(const std::vector<uint8_t>& body,
                                   DisconnectRequestData& data)
{
    (void)body;
    (void)data;
    return true;
}
