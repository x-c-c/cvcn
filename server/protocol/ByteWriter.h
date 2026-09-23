/**
 * @file    ByteWriter.h
 * @brief   Примитивы записи в байтовый буфер с фиксированным порядком байт.
 *
 * @details
 *
 *   Набор функций в namespace ByteWriter. Каждая дописывает
 *   одно значение в конец std::vector<uint8_t>. Никакого состояния,
 *   никакого класса — только функции.
 *
 *   Основное назначение — сериализация полей пакета в network byte
 *   order (big-endian). Именно здесь, в единственном месте, числа
 *   превращаются из host order в сетевой через htons/htonl. Весь
 *   остальной код работает с host order.
 *
 *   Пара к ByteReader: что записали writer-ом, то читает reader.
 *   Порядок вызовов и типы должны совпадать в PacketBuilder и
 *   PacketParser, иначе пакет будет разобран неправильно.
 *
 * @warning writeString пишет длину как uint16_t: строки длиннее
 *          65535 байт обрежутся молча. Для username/password/text
 *          этого хватает с запасом, но стоит помнить.
 * @see     ByteReader, PacketBuilder, PacketParser
 */

#pragma once
#include <vector>
#include <cstdint>
#include <string>

namespace ByteWriter
{
    /**
     * @brief Дописывает count байт из src в конец dest.
     *
     * @param dest  Буфер, в который пишем. Расширяется по необходимости.
     * @param src   Указатель на данные (любой тип, читается как байты).
     * @param count Количество байт для копирования.
     *
     * @details
     *   Расширяет dest на count байт и копирует туда src через memcpy.
     *   Данные не анализируются — просто сырая память, без учёта
     *   выравнивания и типов. Вызывающий отвечает за то, чтобы src
     *   указывал на count валидных байт.
     *
     * @note  Базовая операция для всех writeXxx: они лишь готовят
     *        значение в нужном порядке байт и зовут appendBytes.
     */
    void appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count);

    /**
     * @brief Пишет 1 байт. Порядок байт не важен — байт один.
     * @param buffer Буфер для записи.
     * @param value  Значение (0..255).
     */
    void writeUint8(std::vector<uint8_t>& buffer, uint8_t value);

    /**
     * @brief Пишет 2 байта в big-endian (network order).
     * @param buffer Буфер для записи.
     * @param value  Значение. htons переведёт host → network.
     */
    void writeUint16BE(std::vector<uint8_t>& buffer, uint16_t value);

    /**
     * @brief Пишет 4 байта в big-endian (network order).
     * @param buffer Буфер для записи.
     * @param value  Значение. htonl переведёт host → network.
     */
    void writeUint32BE(std::vector<uint8_t>& buffer, uint32_t value);

    /**
     * @brief Пишет строку с префиксом длины (uint16 BE).
     *
     * @param buffer Буфер для записи.
     * @param str    Строка в UTF-8 (или любые байты).
     *
     * @details
     *   Формат на проводе: [length:2 BE][bytes:length].
     *   Парная функция — ByteReader::readString.
     */
    void writeString(std::vector<uint8_t>& buffer, const std::string& str);
}
