#pragma once
#include "PacketData.h"
#include <vector>

class ClientSession;

/**
 * @file Task.h
 * @brief Задача для обработки одного пакета в воркере.
 *
 * Хранит указатель на сессию, заголовок и тело. Гарантия времени
 * жизни сессии обеспечена SessionManager: пока in-flight счётчик для
 * fd больше нуля, сессия не удаляется.
 */
struct Task
{
    ClientSession* session;
    PacketHeaderRaw header;
    std::vector<uint8_t> body;
};
