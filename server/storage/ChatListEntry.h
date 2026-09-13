#pragma once
#include <cstdint>
#include <string>

/**
 * @file ChatListEntry.h
 * @brief Краткая информация о чате: ID и имя собеседника.
 *
 * Общая структура для хранилища и протокола. Вынесена отдельно,
 * чтобы ChatRepository не зависел от PacketData.h целиком.
 */
struct ChatListEntry
{
    uint32_t chatID;
    std::string peerUsername;
};
