/**
 * @file    ByteReader.cpp
 * @brief   Реализация примитивов чтения.
 *
 * @details
 *   Все функции следуют одному шаблону:
 *     1. Проверить, что remaining хватает на значение.
 *     2. Скопировать байты через memcpy (без выравнивания).
 *     3. Привести к host order (ntohs/ntohl).
 *     4. Сдвинуть cursor и уменьшить remaining.
 *
 *   memcpy вместо reinterpret_cast: избегает UB из-за невыровненного
 *   доступа (данные в буфере могут лежать по любому адресу).
 *
 * @see ByteReader.h
 */

#include "./ByteReader.h"
#include <cstring>
#include <arpa/inet.h>

namespace ByteReader
{
    uint8_t readUint8(const uint8_t*& cursor, size_t& remaining)
    {
        // Проверка границы: если байт остался, читаем; иначе 0.
        if (remaining < sizeof(uint8_t))
            return 0;

        uint8_t value = 0;
        std::memcpy(&value, cursor, sizeof(value));
        cursor += sizeof(uint8_t);
        remaining -= sizeof(uint8_t);
        return value;
    }

    uint16_t readUint16BE(const uint8_t*& cursor, size_t& remaining)
    {
        if (remaining < sizeof(uint16_t))
            return 0;
        uint16_t value = 0;
        std::memcpy(&value, cursor, sizeof(value));

        // ntohs: network (big-endian) → host order. На x86 меняет
        // байты местами, на big-endian — оставляет как есть.
        value = ntohs(value);

        cursor += sizeof(uint16_t);
        remaining -= sizeof(uint16_t);
        return value;
    }

    uint32_t readUint32BE(const uint8_t*& cursor, size_t& remaining)
    {
        if (remaining < sizeof(uint32_t))
            return 0;

        uint32_t value = 0;
        std::memcpy(&value, cursor, sizeof(value));
        value = ntohl(value);

        cursor += sizeof(uint32_t);
        remaining -= sizeof(uint32_t);
        return value;
    }

    std::string readString(const uint8_t*& cursor, size_t& remaining)
    {
        // Сначала читаем префикс длины. Если байт не хватило,
        // readUint16BE вернёт 0 и НЕ сдвинет cursor — remaining
        // останется прежним.
        const uint16_t length = readUint16BE(cursor, remaining);

        // Если заявленная длина больше, чем осталось, — битый пакет.
        // Возвращаем пустую строку, курсор уже сдвинут на префикс
        // длины (readUint16BE это сделал).
        if (remaining < length)
            return "";

        // Создаём строку из length байт. reinterpret_cast — потому что
        // char и uint8_t могут быть разными типами (signed vs unsigned),
        // но байты те же.
        std::string str(reinterpret_cast<const char*>(cursor), length);

        cursor += length;
        remaining -= length;
        return str;
    }
}
