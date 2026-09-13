#pragma once
#include <string>
#include <sqlite3.h>
class Database
{
public:
	explicit Database(const std::string& dbPath);
	~Database();

	Database(const Database&) = delete;
	Database& operator=(const Database&) = delete;

	/** @brief Сырой указатель на соединение. Используется репозиториями. */
	sqlite3* getHandle() const { return db_; }

private:
	sqlite3* db_ = nullptr;

	bool open(const std::string& dbPath);
	bool createTables();
	bool executeSql(const char* sql, const char* context);
};
