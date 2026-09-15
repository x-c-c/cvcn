/**
 * @file    PacketAssembler.h
 * @brief   Накопитель входящих байтов: собирает целые пакеты из потока.
 *
 * @details
 *   TCP — поток байтов, а не последовательность сообщений: один recv
 *   может вернуть полтора пакета, три пакета или половину. Задача
 *   PacketAssembler — копить байты в буфере и отдавать вызывающему
 *   ровно один целый пакет за раз, как только он полностью пришёл.
 *     - appendData()  — докинуть байты из recv в конец буфера;
 *     - extractPacket() — если в буфере есть полный пакет, вырезать
 *                         его и вернуть через out-параметры, иначе
 *                         вернуть false и оставить буфер как есть.
 *
 *   Вызывающий код обычно крутит цикл:
 *     while (assembler.extractPacket(header, body)) { processPacket(...); }
 *
 *   Класс не знает ни про сокет, ни про PacketType — он работает
 *   только с сырыми байтами и разбирает минимальный заголовок, чтобы
 *   понять длину. Всё остальное — дело PacketParser.
 *
 * @warning MAX_BUFFER_SIZE объявлен, но в текущей реализации не
 *          проверяется. Если клиент пришлёт messageLen = 60000 и
 *          оборвёт соединение, буфер будет висеть до закрытия сессии.
 *          Стоит добавить проверку при появлении реальной нагрузки.
 * @see     ClientSession, PacketParser, PacketBuilder
 */

#pragma once
#include "./PacketData.h"
#include <vector>
#include <cstdint>

class PacketAssembler
{
public:
    /**
     * @brief Добавляет байты в конец внутреннего буфера.
     *
     * @param data Указатель на байты (обычно tempBuffer в handleRead).
     * @param len  Количество байт.
     *
     * @details
     *   Расширяет readBuffer_ на len байт и копирует данные в хвост.
     *   Данные не анализируются — просто накопление.
     */
    void appendData(const uint8_t* data, size_t len);

    /**
     * @brief Извлекает один целый пакет из буфера, если он там есть.
     *
     * @param header Выходной параметр: разобранный заголовок (host order).
     * @param body   Выходной параметр: тело пакета — байты после заголовка.
     *
     * @return true  — пакет извлечён, буфер сдвинут к следующему пакету;
     *         false — байт пока не хватает (ни заголовка, ни тела),
     *                 буфер не изменён.
     *
     * @details
     *   Алгоритм:
     *     1. Если в буфере меньше sizeof(PacketHeaderRaw) — вернуть false.
     *     2. Скопировать первые 12 байт в header, развернуть byte order.
     *     3. Посчитать totalSize = 12 + header.messageLen.
     *     4. Если буфер меньше totalSize — вернуть false (ждём ещё).
     *     5. Вырезать body, стереть из буфера первые totalSize байт.
     *
     * @note    Заголовок разбирается здесь, а не через PacketParser,
     *          потому что assembler-у нужна только длина тела. Полный
     *          разбор делает PacketParser::deserializeData позже.
     */
    bool extractPacket(PacketHeaderRaw& header, std::vector<uint8_t>& body);

private:
    std::vector<uint8_t> readBuffer_;   ///< Владеет. Накопленные байты.

    /**
     * @brief Верхняя граница размера буфера.
     * @note  64 КБ — компромисс между памятью на сессию и максимальным
     *        разумным пакетом (текст сообщения + запас).
     */
    static constexpr size_t MAX_BUFFER_SIZE = 64 * 1024;
};
