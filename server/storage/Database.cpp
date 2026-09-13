#include "Database.h"
#include "Logger.h"
#include <stdexcept>

Database::Database(const std::string& dbPath)
{
	if (!open(dbPath))
		throw std::runtime_error("Failed to open database: " + dbPath);
}

Database::~Database()
{
	if (db_)
	{
		sqlite3_close(db_);
		db_ = nullptr;
	}
}

bool Database::executeSql(const char* sql, const char* context)
{
	char* errMsg = nullptr;
	const int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK)
	{
		Logger::instance().error("SQL error ({}): {}", context, errMsg ? errMsg : "unknown");
		if (errMsg)
			sqlite3_free(errMsg);
		return false;
	}
	return true;
}

bool Database::open(const std::string& dbPath)
{
	const int rc = sqlite3_open(dbPath.c_str(), &db_);
	if (rc != SQLITE_OK)
	{
		Logger::instance().error("Cannot open database: {}", sqlite3_errmsg(db_));
		return false;
	}

	sqlite3_exec(db_, "PRAGMA journal_mode=WAL;",  nullptr, nullptr, nullptr);
	sqlite3_exec(db_, "PRAGMA foreign_keys=ON;",   nullptr, nullptr, nullptr);

	return createTables();
}

bool Database::createTables()
{
	const char* usersSql =
		"CREATE TABLE IF NOT EXISTS users ("
		"  id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  username TEXT NOT NULL UNIQUE,"
		"  password_hash TEXT NOT NULL,"
		"  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
		");";

	const char* chatsSql =
		"CREATE TABLE IF NOT EXISTS chats ("
		"  id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  is_group INTEGER NOT NULL DEFAULT 0,"
		"  name TEXT,"
		"  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
		");";

	const char* membersSql =
		"CREATE TABLE IF NOT EXISTS chat_members ("
		"  chat_id INTEGER NOT NULL,"
		"  user_id INTEGER NOT NULL,"
		"  PRIMARY KEY (chat_id, user_id)"
		");";

	const char* messagesSql =
		"CREATE TABLE IF NOT EXISTS messages ("
		"  id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  chat_id INTEGER NOT NULL,"
		"  sender_id INTEGER NOT NULL,"
		"  text TEXT NOT NULL,"
		"  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
		");";

	if (!executeSql(usersSql,    "create users"))        return false;
	if (!executeSql(chatsSql,    "create chats"))        return false;
	if (!executeSql(membersSql,  "create chat_members")) return false;
	if (!executeSql(messagesSql, "create messages"))     return false;

	return true;
}
