#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>
#include "PacketData.h"

class Database
{
private:
	sqlite3* db_ = nullptr;
	bool open(const std::string& dbPath);
	bool executeSql(const char* sql, const char* context);

public:
	explicit Database(const std::string& dbPath);
	~Database();

	// Пользователи
	bool isUserExist(const std::string& username);
	bool addUser(const std::string& username, const std::string& passwordHash);
	bool deleteUser(const std::string& username, const std::string& passwordHash);

	std::string getUserPasswordHash(const std::string& username);
	int getUserID(const std::string& username);

	// Поиск и чаты
	std::vector<std::string> findUsers(const std::string& query, int excludeUserID);
	int findOrCreateDirectChat(int user1ID, int user2ID);
	std::vector<ChatListEntry> getUserChats(int userID);
};
