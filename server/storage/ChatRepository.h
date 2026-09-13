#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <sqlite3.h>
#include "ChatListEntry.h"
#include "IChatRepository.h"

class ChatRepository : public IChatRepository
{
public:
    explicit ChatRepository(sqlite3* db);

    std::vector<std::string> findUsers(const std::string& query, int excludeUserID) override;
    int findOrCreateDirectChat(int user1ID, int user2ID) override;
    std::vector<ChatListEntry> getUserChats(int userID) override;
    std::vector<int> getChatMemberIDs(uint32_t chatID) override;

private:
    sqlite3* db_;
};
