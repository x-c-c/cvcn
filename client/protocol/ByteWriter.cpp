/**
 * @file    ByteWriter.cpp
 * @brief   Реализация примитивов записи.
 *
 * @details
 *   Все функции строятся вокруг appendBytes: он расширяет вектор
 *   и копирует байты. writeUint16BE/writeUint32BE сначала конвертируют
 *   значение в network order, потом вызывают appendBytes.
 *
 *   Порядок байт задаётся один раз здесь и навсегда фиксируется
 *   в протоколе. ByteReader разворачивает ровно те же типы.
 *
 * @see ByteWriter.h
 */

#include "./ByteWriter.h"
#include <cstring>
#include <arpa/inet.h>

namespace ByteWriter
{
    void appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count)
    {
        // Запоминаем старый размер, расширяем и копируем в хвост.
        // resize не инициализирует — memcpy сразу перезаписывает.
        const size_t offset = dest.size();
        dest.resize(offset + count);
        std::memcpy(dest.data() + offset, src, count);
    }

    void writeUint8(std::vector<uint8_t>& buffer, uint8_t value)
    {
        // Байт — порядок не важен.
        appendBytes(buffer, &value, sizeof(value));
    }

    void writeUint16BE(std::vector<uint8_t>& buffer, uint16_t value)
    {
        // htons: на x86 меняет байты местами, на big-endian — no-op.
        // После него память &networkOrder содержит [hi][lo].
        const uint16_t networkOrder = htons(value);
        appendBytes(buffer, &networkOrder, sizeof(networkOrder));
    }

    void writeUint32BE(std::vector<uint8_t>& buffer, uint32_t value)
    {
        // Аналогично writeUint16BE, но 4 байта и htonl.
        const uint32_t networkOrder = htonl(value);
        appendBytes(buffer, &networkOrder, sizeof(networkOrder));
    }

    void writeString(std::vector<uint8_t>& buffer, const std::string& str)
    {
        // Формат: [length:2 BE][bytes:length]. Длина — это размер
        // строки в байтах, не символов (для UTF-8 они разные).
        const uint16_t length = static_cast<uint16_t>(str.size());

        // Префикс длины пишется big-endian, как и остальные числа.
        writeUint16BE(buffer, length);

        // Сама строка — как есть, без завершающего нуля.
        appendBytes(buffer, str.data(), length);
    }
}
