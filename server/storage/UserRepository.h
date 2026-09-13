#pragma once
#include <string>
#include <sqlite3.h>
class UserRepository
{
public:
	explicit UserRepository(sqlite3* db);

	bool isUserExist(const std::string& username);
	bool addUser(const std::string& username, const std::string& passwordHash);
	bool deleteUser(const std::string& username, const std::string& passwordHash);

	std::string getUserPasswordHash(const std::string& username);
	int getUserID(const std::string& username);

private:
	sqlite3* db_;
};
