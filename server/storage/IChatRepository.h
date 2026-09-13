#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "ChatListEntry.h"

/**
 * @file IChatRepository.h
 * @brief Интерфейс доступа к таблицам chats и chat_members.
 */
class IChatRepository
{
public:
    virtual ~IChatRepository() = default;

    virtual std::vector<std::string> findUsers(const std::string& query, int excludeUserID) = 0;
    virtual int findOrCreateDirectChat(int user1ID, int user2ID) = 0;
    virtual std::vector<ChatListEntry> getUserChats(int userID) = 0;
    virtual std::vector<int> getChatMemberIDs(uint32_t chatID) = 0;
};
