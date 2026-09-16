/**
 * @file    PacketParser.h
 * @brief   Разбор байтов в структуры данных протокола.
 *
 * @details
 *   Набор статических функций, обратных PacketBuilder: превращают
 *   тело пакета (сырые байты после заголовка) в конкретную Data-структуру.
 *
 *   Два уровня разбора:
 *     - deserializeHeader — разбирает 12-байтовый PacketHeaderRaw,
 *       переводит поля из network byte order в host;
 *     - deserializeData   — одна перегрузка на каждый тип тела,
 *       читает поля через ByteReader.
 *
 *   Перегрузка по типу Data — это и есть диспетчеризация: вызывающий
 *   код в ClientSession::processPacket объявляет нужную структуру
 *   и получает её заполненной, если тело корректно.
 * @see     PacketBuilder, ByteReader, PacketData, ClientSession
 */

#pragma once

#include "./PacketData.h"
#include <cstdint>
#include <vector>

class PacketParser
{
public:
    /**
     * @brief Разбирает заголовок пакета из буфера.
     *
     * @param rawData Буфер, начиная с заголовка. Может быть больше —
     *                функция читает только первые sizeof(PacketHeaderRaw)
     *                байт, остальное игнорируется.
     * @param header  Выходная структура. Заполняется только при true.
     *
     * @return true  — заголовок разобран, поля в host byte order;
     *         false — в rawData меньше sizeof(PacketHeaderRaw) байт.
     *
     * @details
     *   Копирует 12 байт из rawData в header, затем разворачивает
     *   многобайтовые поля из network order:
     *     type       — ntohs (2 байта);
     *     messageID  — ntohl (4 байта);
     *     sessionID  — ntohl (4 байта);
     *     messageLen — ntohs (2 байта).
     *
     * @note Структура PacketHeaderRaw объявлена с #pragma pack(1),
     *       поэтому sizeof == 12 без выравнивания.
     */
    static bool deserializeHeader(const std::vector<uint8_t>& rawData,
                                  PacketHeaderRaw& header);

    //  Разбор тел пакетов 
    //   @param body  Сырые байты тела (после заголовка). Длина должна
    //                точно совпадать с суммой размеров полей.
    //   @param data  Выходная структура. Заполняется только при true.
    //   @return true  — тело разобрано, remaining == 0;
    //           false — байт не хватило или остались лишние.
    //
    // Внутри каждая перегрузка идёт курсором по body через ByteReader,
    // в конце проверяет, что курсор дошёл ровно до конца. Если
    // remaining != 0 — тело содержит лишние байты, это ошибка формата.

    /// @brief Тело пустое. Всегда true.
    static bool deserializeData(const std::vector<uint8_t>& body,
                                ConnectRequestData& data);

    /// @brief Тело пустое. Всегда true.
    static bool deserializeData(const std::vector<uint8_t>& body,
                                ConnectResponseData& data);

    /// @brief Регистрация: username + password (две строки).
    static bool deserializeData(const std::vector<uint8_t>& body,
                                RegisterRequestData& data);

    /// @brief Ответ регистрации: success (1 байт).
    static bool deserializeData(const std::vector<uint8_t>& body,
                                RegisterResponseData& data);

    /// @brief Аутентификация: username + password (две строки).
    static bool deserializeData(const std::vector<uint8_t>& body,
                                AuthRequestData& data);

    /// @brief Ответ аутентификации: success (1 байт).
    static bool deserializeData(const std::vector<uint8_t>& body,
                                AuthResponseData& data);

    /// @brief Сообщение: senderID + chatID + text.
    static bool deserializeData(const std::vector<uint8_t>& body,
                                MessageSendData& data);

    /// @brief Тело пустое. Всегда true.
    static bool deserializeData(const std::vector<uint8_t>& body,
                                DisconnectRequestData& data);
};
