#pragma once
#include <string>
#include <sqlite3.h>

class Database
{
private:
	sqlite3* db_ = nullptr;
	bool open(const std::string& dbPath);

public:
	explicit Database(const std::string& dbPath);
	~Database();

	// Пользователи
	bool isUserExist(const std::string& username);
	bool addUser(const std::string& username, const std::string& passwordHash);
	
	std::string getUserPasswordHash(const std::string& username);
	int getUserID(const std::string& username);
	
	// Чаты
	int createChat(bool isGroup, const std::string& name);
};
