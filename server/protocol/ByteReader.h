/**
 * @file    ByteReader.h
 * @brief   Примитивы чтения из байтового буфера с фиксированным порядком байт.
 *
 * @details
 *
 *   Набор функций в namespace ByteReader. Пара к ByteWriter:
 *   что writer записал в big-endian, reader читает в том же порядке
 *   и возвращает значение в host order. Никакого класса, никакого
 *   состояния — только функции.
 *
 *   Все функции принимают два параметра по ссылке:
 *     - cursor    — указатель на текущую позицию в буфере;
 *     - remaining — сколько байт осталось до конца.
 *
 *   Функция читает значение, сдвигает cursor вперёд и уменьшает
 *   remaining. Это позволяет вызывающему коду последовательно
 *   разобрать несколько полей, не заводя собственный счётчик.
 *
 *   При нехватке байт функция возвращает 0
 *   (или пустую строку) и НЕ двигает курсор. Проверка корректности —
 *   на стороне вызывающего (PacketParser сверяет remaining == 0
 *   в конце разбора).
 *
 * @note    Порядок байт всегда network (big-endian). Это соответствует
 *          тому, что пишет ByteWriter, и стандарту большинства сетевых
 *          протоколов.
 * @see     ByteWriter, PacketParser, PacketAssembler
 */

#pragma once
#include <cstdint>
#include <string>

namespace ByteReader
{
    /**
     * @brief Читает 1 байт.
     *
     * @param cursor    Указатель на текущую позицию. Сдвигается на 1.
     * @param remaining Остаток байт. Уменьшается на 1.
     * @return Значение байта или 0 при нехватке данных.
     *
     * @note Порядок байт не важен — байт один.
     */
    uint8_t readUint8(const uint8_t*& cursor, size_t& remaining);

    /**
     * @brief Читает 2 байта в big-endian.
     *
     * @param cursor    Указатель на текущую позицию. Сдвигается на 2.
     * @param remaining Остаток байт. Уменьшается на 2.
     * @return Значение в host order или 0 при нехватке данных.
     *
     * @details
     *   Копирует 2 байта в uint16_t (little-endian на x86), затем
     *   ntohs меняет байты на host order. На big-endian машинах
     *   ntohs — no-op.
     */
    uint16_t readUint16BE(const uint8_t*& cursor, size_t& remaining);

    /**
     * @brief Читает 4 байта в big-endian.
     *
     * @param cursor    Указатель на текущую позицию. Сдвигается на 4.
     * @param remaining Остаток байт. Уменьшается на 4.
     * @return Значение в host order или 0 при нехватке данных.
     *
     * @details
     *   Аналогично readUint16BE, но 4 байта и ntohl.
     */
    uint32_t readUint32BE(const uint8_t*& cursor, size_t& remaining);

    /**
     * @brief Читает строку с префиксом длины (uint16 BE).
     *
     * @param cursor    Указатель на текущую позицию.
     * @param remaining Остаток байт.
     * @return Строка или пустая строка при нехватке данных.
     *
     * @details
     *   Формат на проводе: [length:2 BE][bytes:length].
     *   Парная функция — ByteWriter::writeString.
     *
     *   Сначала читает 2 байта длины через readUint16BE. Затем
     *   проверяет, что remaining >= length. Если да — создаёт
     *   std::string из следующих length байт и сдвигает курсор.
     *
     * @warning Возвращает пустую строку и при length == 0, и при
     *          нехватке байт. Отличить эти случаи по возвращаемому
     *          значению нельзя — проверяйте remaining.
     */
    std::string readString(const uint8_t*& cursor, size_t& remaining);
}
