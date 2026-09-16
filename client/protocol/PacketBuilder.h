/**
 * @file    PacketBuilder.h
 * @brief   Сериализация структур протокола в готовые к отправке байты.
 *
 * @details
 *
 *   Набор статических функций, обратных PacketParser: превращают
 *   Data-структуру в std::vector<uint8_t> — полный пакет с заголовком
 *   и телом, готовый к отправке в сокет.
 *
 *   Одна публичная перегрузка на каждый тип Data. Перегрузка выбирается
 *   компилятором по типу третьего аргумента, поэтому вызывающему коду
 *   не нужно вручную указывать PacketType — тип пакета в заголовке
 *   проставляется автоматически.
 *
 *   Формат на проводе (все многобайтовые числа — big-endian):
 *     [type:2][messageID:4][sessionID:4][messageLen:2][body:messageLen]
 *
 * @warning Поля username/password/text передаются без шифрования.
 * @see     PacketParser, ByteWriter, PacketData, ClientSession
 */

#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H
#include <vector>
#include <cstdint>
#include "./PacketData.h"

class PacketBuilder
{
private:
    /**
     * @brief Базовый сборщик: заголовок + тело.
     *
     * @param type      Тип пакета (значение PacketType).
     * @param messageID Уникальный ID запроса в рамках сессии.
     * @param sessionID ID сессии (0 — до ConnectRequest).
     * @param body      Готовое тело пакета. По умолчанию пусто —
     *                  для пакетов без тела (Connect, Disconnect).
     *
     * @return Полный пакет: 12-байтовый заголовок + body.
     *
     * @details
     *   Заполняет PacketHeaderRaw, конвертирует поля в network order
     *   (htons/htonl), склеивает заголовок и тело через ByteWriter.
     *   Приватный — вызывать только из публичных перегрузок, которые
     *   знают, какой PacketType подставить.
     *
     * @note messageLen конвертируется в uint16_t: тело не может быть
     *       длиннее 65535 байт. Для текущего протокола этого хватает
     *       с запасом, но при появлении больших пакетов (файлы,
     *       длинные истории) тип поля придётся менять на uint32_t.
     */
    static std::vector<uint8_t> buildPacket(PacketType type,
                                            uint32_t messageID,
                                            uint32_t sessionID,
                                            const std::vector<uint8_t>& body = {});

public:
    //  Публичные перегрузки: по одной на каждый тип Data 
    // Все возвращают готовый к отправке пакет. Тело собирается из
    // полей Data через ByteWriter, затем базовый buildPacket добавляет
    // 12-байтовый заголовок.

    /// @brief ConnectRequest: тело пустое.
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID,
                                            const ConnectRequestData&);

    /// @brief ConnectResponse: тело пустое, sessionID из аргумента.
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID,
                                            const ConnectResponseData&);

    /// @brief AuthRequest: username + password двумя строками.
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID,
                                            const AuthRequestData&);

    /// @brief AuthResponse: success (1 байт).
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID,
                                            const AuthResponseData&);

    /// @brief RegisterRequest: username + password двумя строками.
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID,
                                            const RegisterRequestData&);

    /// @brief RegisterResponse: success (1 байт).
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID,
                                            const RegisterResponseData&);

    /// @brief MessageSend: senderID + chatID + text.
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID,
                                            const MessageSendData&);

    /// @brief DisconnectRequest: тело пустое.
    static std::vector<uint8_t> buildPacket(uint32_t messageID, uint32_t sessionID,
                                            const DisconnectRequestData&);
};

#endif // PACKETBUILDER_H
