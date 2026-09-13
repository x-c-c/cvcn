#pragma once
#include <cstdint>
#include <string>
#include <sqlite3.h>

class MessageRepository
{
public:
	explicit MessageRepository(sqlite3* db);

	bool saveMessage(uint32_t chatID, int senderID, const std::string& text);

private:
	sqlite3* db_;
};
