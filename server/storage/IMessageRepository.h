#pragma once
#include <cstdint>
#include <string>

/**
 * @file IMessageRepository.h
 * @brief Интерфейс доступа к таблице messages.
 */
class IMessageRepository
{
public:
    virtual ~IMessageRepository() = default;

    virtual bool saveMessage(uint32_t chatID, int senderID, const std::string& text) = 0;
};
