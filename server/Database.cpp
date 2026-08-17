#include "Database.h"
#include "Logger.h"
#include <stdexcept>
#include <cstring>

Database::Database(const std::string& dbPath)
{
	if (!open(dbPath))
		throw std::runtime_error("Failed to open database: " + dbPath);
}

Database::~Database()
{
	if (db_)
		sqlite3_close(db_);
}

bool Database::open(const std::string& dbPath)
{
	int returnCode = sqlite3_open(dbPath.c_str(), &db_);
	if (returnCode != SQLITE_OK)
	{
		Logger::instance().error("Cannot open database: {}", sqlite3_errmsg(db_));
		return false;
	}
	sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

	const char* sql =
		"CREATE TABLE IF NOT EXISTS users ("
		"  id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  username TEXT NOT NULL UNIQUE,"
		"  password_hash TEXT NOT NULL,"
		"  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
		");";

	char* errMsg = nullptr;
	returnCode = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
	if (returnCode != SQLITE_OK)
	{
		Logger::instance().error("SQL error: {}", errMsg);
		sqlite3_free(errMsg);
		return false;
	}
	return true;
}

bool Database::addUser(const std::string& username, const std::string& passwordHash)
{
	const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_STATIC);

	int returnCode = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (returnCode != SQLITE_DONE)
	{
		Logger::instance().error("Failed to add user '{}': {}", username, sqlite3_errmsg(db_));
		return false;
	}
	return true;
}

std::string Database::getUserPasswordHash(const std::string& username)
{
	const char* sql = "SELECT password_hash FROM users WHERE username = ?";
	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

	std::string hash;
	if (sqlite3_step(stmt) == SQLITE_ROW)
		hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);
	return hash;
}

bool Database::isUserExist(const std::string& username)
{
	return !getUserPasswordHash(username).empty();
}

int Database::getUserID(const std::string& username)
{
	const char* sql = "SELECT id FROM users WHERE username = ?";
	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

	int userID = -1;
	if (sqlite3_step(stmt) == SQLITE_ROW)
		userID = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);
	return userID;
}




