#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <sqlite3.h>
#include "PacketData.h"

class ChatRepository
{
public:
	explicit ChatRepository(sqlite3* db);

	std::vector<std::string> findUsers(const std::string& query, int excludeUserID);
	int findOrCreateDirectChat(int user1ID, int user2ID);
	std::vector<ChatListEntry> getUserChats(int userID);
	std::vector<int> getChatMemberIDs(uint32_t chatID);

private:
	sqlite3* db_;
};
