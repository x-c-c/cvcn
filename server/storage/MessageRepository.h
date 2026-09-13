#pragma once
#include <cstdint>
#include <string>
#include <sqlite3.h>
#include "IMessageRepository.h"

class MessageRepository : public IMessageRepository
{
public:
    explicit MessageRepository(sqlite3* db);

    bool saveMessage(uint32_t chatID, int senderID, const std::string& text) override;

private:
    sqlite3* db_;
};
