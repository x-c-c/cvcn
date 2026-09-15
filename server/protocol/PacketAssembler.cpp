/**
 * @file    PacketAssembler.cpp
 * @brief   Реализация PacketAssembler.
 *
 * @details
 *   readBuffer_ — это «хвост» потока, который ещё не превратился
 *   в целый пакет. appendData копит, extractPacket вырезает готовое.
 *
 * @see PacketAssembler.h
 */

#include "./PacketAssembler.h"
#include <cstring>
#include <arpa/inet.h>

void PacketAssembler::appendData(const uint8_t* data, size_t len)
{
    // Запоминаем старый размер, увеличиваем буфер на len и копируем
    // новые байты в хвост. resize не инициализирует нулями — просто
    // выделяет память, memcpy сразу перезапишет.
    const size_t oldSize = readBuffer_.size();
    readBuffer_.resize(oldSize + len);
    std::memcpy(readBuffer_.data() + oldSize, data, len);
}

bool PacketAssembler::extractPacket(PacketHeaderRaw& header, std::vector<uint8_t>& body)
{
    // Шаг 1: нужен минимум заголовок. Если его нет — ждём ещё байты.
    if (readBuffer_.size() < sizeof(PacketHeaderRaw))
        return false;

    // Шаг 2: копируем 12 байт как есть (структура упакована pack(1),
    // поэтому memcpy в неё безопасен). Затем разворачиваем поля
    // из network в host order — те же операции, что в
    // PacketParser::deserializeHeader, но здесь они нужны для
    // messageLen, иначе не посчитать длину тела.
    std::memcpy(&header, readBuffer_.data(), sizeof(header));
    header.type       = ntohs(header.type);
    header.messageID  = ntohl(header.messageID);
    header.sessionID  = ntohl(header.sessionID);
    header.messageLen = ntohs(header.messageLen);

    // Шаг 3: полный размер пакета. messageLen — это длина тела,
    // не всего пакета, поэтому прибавляем размер заголовка.
    const size_t totalSize = sizeof(PacketHeaderRaw) + header.messageLen;

    // Шаг 4: если тела ещё не хватает — возвращаем false.
    // ВАЖНО: header при этом уже заполнен, но вызывающий код должен
    // полагаться только на возвращаемое значение, а не на header.
    if (readBuffer_.size() < totalSize)
        return false;

    // Шаг 5: вырезаем тело в отдельный vector и стираем пакет из
    // буфера. erase сдвигает оставшиеся байты в начало — это O(n),
    // где n — размер «хвоста». Для мелких пакетов не критично,
    // но при большом потоке стоит перейти на read-offset.
    body.assign(readBuffer_.begin() + sizeof(PacketHeaderRaw),
                readBuffer_.begin() + totalSize);
    readBuffer_.erase(readBuffer_.begin(), readBuffer_.begin() + totalSize);
    return true;
}
