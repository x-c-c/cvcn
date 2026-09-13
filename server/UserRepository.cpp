#include "UserRepository.h"
#include "Logger.h"

UserRepository::UserRepository(sqlite3* db): db_(db){}

bool UserRepository::addUser(const std::string& username, const std::string& passwordHash)
{
	const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		Logger::instance().error("prepare failed: {}", sqlite3_errmsg(db_));
		return false;
	}
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_STATIC);

	const int rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (rc != SQLITE_DONE)
	{
		Logger::instance().error("Failed to add user '{}': {}", username, sqlite3_errmsg(db_));
		return false;
	}
	return true;
}

bool UserRepository::deleteUser(const std::string& username, const std::string& passwordHash)
{
	const char* sql = "DELETE FROM users WHERE username = ? AND password_hash = ?";
	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		Logger::instance().error("prepare failed: {}", sqlite3_errmsg(db_));
		return false;
	}
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_STATIC);

	const int rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (rc != SQLITE_DONE)
	{
		Logger::instance().error("Failed to delete user '{}': {}", username, sqlite3_errmsg(db_));
		return false;
	}
	return sqlite3_changes(db_) > 0;
}

std::string UserRepository::getUserPasswordHash(const std::string& username)
{
	const char* sql = "SELECT password_hash FROM users WHERE username = ?";
	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		Logger::instance().error("prepare failed: {}", sqlite3_errmsg(db_));
		return "";
	}
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

	std::string hash;
	if (sqlite3_step(stmt) == SQLITE_ROW)
		hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);
	return hash;
}

bool UserRepository::isUserExist(const std::string& username)
{
	return !getUserPasswordHash(username).empty();
}

int UserRepository::getUserID(const std::string& username)
{
	const char* sql = "SELECT id FROM users WHERE username = ?";
	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		Logger::instance().error("prepare failed: {}", sqlite3_errmsg(db_));
		return -1;
	}
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

	int userID = -1;
	if (sqlite3_step(stmt) == SQLITE_ROW)
		userID = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);
	return userID;
}
