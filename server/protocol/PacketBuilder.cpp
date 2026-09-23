/**
 * @file    PacketBuilder.cpp
 * @brief   Реализация PacketBuilder.
 *
 * @details
 *   Все публичные перегрузки следуют одному шаблону:
 *     1. Собрать тело пакета в локальный vector<uint8_t> через ByteWriter.
 *     2. Вызвать приватный buildPacket(type, messageID, sessionID, body).
 *     3. Вернуть результат.
 *
 *   Для пакетов без тела (Connect, Disconnect) шаг 1 пропускается —
 *   сразу вызывается buildPacket с пустым body по умолчанию.
 *
 *   Приватный buildPacket заполняет PacketHeaderRaw и конвертирует
 *   многобайтовые поля в network byte order через htons/htonl. Это
 *   единственное место в проекте, где числа превращаются из host
 *   в network — вся остальная логика работает с host order.
 *
 * @see PacketBuilder.h
 */

#include "./PacketBuilder.h"
#include "./ByteWriter.h"
#include <cstring>
#include <arpa/inet.h>

std::vector<uint8_t> PacketBuilder::buildPacket(PacketType type,
                                                uint32_t messageID,
                                                uint32_t sessionID,
                                                const std::vector<uint8_t>& body)
{
    // Заголовок заполняется в network byte order сразу, а не после.
    // htons/htonl — no-op на big-endian машинах и byte swap на
    // little-endian (x86, ARM). Один и тот же код работает везде.
    PacketHeaderRaw header;
    header.type       = htons(static_cast<uint16_t>(type));
    header.messageID  = htonl(messageID);
    header.sessionID  = htonl(sessionID);
    header.messageLen = htons(static_cast<uint16_t>(body.size()));

    // Склеиваем заголовок и тело в один буфер. ByteWriter::appendBytes
    // расширяет vector и копирует байты — без выравнивания, как есть.
    std::vector<uint8_t> result;
    ByteWriter::appendBytes(result, &header, sizeof(header));
    ByteWriter::appendBytes(result, body.data(), body.size());
    return result;
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID,
                                                const ConnectRequestData&)
{
    // Тело пустое: клиент сообщает только «хочу подключиться»,
    // sessionID пока 0. Сервер присвоит его в ConnectResponse.
    return buildPacket(PacketType::ConnectRequest, messageID, sessionID);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID,
                                                const ConnectResponseData&)
{
    // sessionID в заголовке — это тот ID, который сервер назначил
    // сессии. Тело пустое, потому что вся информация уже в заголовке.
    return buildPacket(PacketType::ConnectResponse, messageID, sessionID);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID,
                                                const RegisterRequestData& data)
{
    // Тело: [len:2][username][len:2][password].
    std::vector<uint8_t> body;
    ByteWriter::writeString(body, data.username);
    ByteWriter::writeString(body, data.password);
    return buildPacket(PacketType::RegisterRequest, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID,
                                                const RegisterResponseData& data)
{
    // Тело: [success:1]. uint8_t
    std::vector<uint8_t> body;
    ByteWriter::writeUint8(body, data.success);
    return buildPacket(PacketType::RegisterResponse, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID,
                                                const AuthRequestData& data)
{
    // Тело идентично RegisterRequest: две строки с префиксом длины.
    std::vector<uint8_t> body;
    ByteWriter::writeString(body, data.username);
    ByteWriter::writeString(body, data.password);
    return buildPacket(PacketType::AuthRequest, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID,
                                                const AuthResponseData& data)
{
    // Тело: [success:1]. Симметрично RegisterResponse.
    std::vector<uint8_t> body;
    ByteWriter::writeUint8(body, data.success);
    return buildPacket(PacketType::AuthResponse, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID,
                                                const MessageSendData& data)
{
    // Тело: [senderID:4][chatID:4][len:2][text].
    // Порядок и типы должны совпадать с MessageSendData на сервере,
    // иначе PacketParser не разберёт пакет.
    std::vector<uint8_t> body;
    ByteWriter::writeUint32BE(body, data.senderID);
    ByteWriter::writeUint32BE(body, data.chatID);
    ByteWriter::writeString(body, data.text);
    return buildPacket(PacketType::MessageSend, messageID, sessionID, body);
}

std::vector<uint8_t> PacketBuilder::buildPacket(uint32_t messageID, uint32_t sessionID,
                                                const DisconnectRequestData&)
{
    // Тело пустое. Клиент сообщает серверу о намерении отключиться,
    // чтобы сервер мог корректно закрыть сессию до EOF на сокете.
    return buildPacket(PacketType::DisconnectRequest, messageID, sessionID);
}
