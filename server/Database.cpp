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
	int returnCode = sqlite3_open(dbPath.c_str(), &db_);
	if (returnCode != SQLITE_OK)
	{
		Logger::instance().error("Cannot open database: {}", sqlite3_errmsg(db_));
		return false;
	}
	sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
	sqlite3_exec(db_, "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr);

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

	if (!executeSql(usersSql, "create users"))       return false;
	if (!executeSql(chatsSql, "create chats"))       return false;
	if (!executeSql(membersSql, "create chat_members")) return false;
	if (!executeSql(messagesSql, "create messages")) return false;

	return true;
}

bool Database::addUser(const std::string& username, const std::string& passwordHash)
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

	int returnCode = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (returnCode != SQLITE_DONE)
	{
		Logger::instance().error("Failed to add user '{}': {}", username, sqlite3_errmsg(db_));
		return false;
	}
	return true;
}

bool Database::deleteUser(const std::string& username, const std::string& passwordHash)
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

	int returnCode = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (returnCode != SQLITE_DONE)
	{
		Logger::instance().error("Failed to delete user '{}': {}", username, sqlite3_errmsg(db_));
		return false;
	}
	return sqlite3_changes(db_) > 0;
}

std::string Database::getUserPasswordHash(const std::string& username)
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

bool Database::isUserExist(const std::string& username)
{
	return !getUserPasswordHash(username).empty();
}

int Database::getUserID(const std::string& username)
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

std::vector<std::string> Database::findUsers(const std::string& query, int excludeUserID)
{
	std::vector<std::string> result;
	const char* sql =
		"SELECT username FROM users "
		"WHERE username LIKE ? AND id != ? "
		"ORDER BY username ASC LIMIT 20";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		Logger::instance().error("prepare failed: {}", sqlite3_errmsg(db_));
		return result;
	}

	std::string pattern = query + "%";
	sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, excludeUserID);

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		if (name)
			result.emplace_back(name);
	}
	sqlite3_finalize(stmt);
	return result;
}

int Database::findOrCreateDirectChat(int user1ID, int user2ID)
{
	const char* findSql =
		"SELECT c.id FROM chats c "
		"JOIN chat_members m1 ON m1.chat_id = c.id AND m1.user_id = ? "
		"JOIN chat_members m2 ON m2.chat_id = c.id AND m2.user_id = ? "
		"WHERE c.is_group = 0 "
		"AND (SELECT COUNT(*) FROM chat_members WHERE chat_id = c.id) = 2 "
		"LIMIT 1";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db_, findSql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		Logger::instance().error("prepare failed: {}", sqlite3_errmsg(db_));
		return -1;
	}
	sqlite3_bind_int(stmt, 1, user1ID);
	sqlite3_bind_int(stmt, 2, user2ID);

	int chatID = -1;
	if (sqlite3_step(stmt) == SQLITE_ROW)
		chatID = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);
	if (chatID != -1)
		return chatID;

	// Создаём новый чат
	if (!executeSql("BEGIN", "begin tx"))
		return -1;

	const char* insertChat = "INSERT INTO chats (is_group, name) VALUES (0, NULL)";
	if (!executeSql(insertChat, "insert chat"))
	{
		executeSql("ROLLBACK", "rollback");
		return -1;
	}
	chatID = static_cast<int>(sqlite3_last_insert_rowid(db_));

	const char* insertMember = "INSERT INTO chat_members (chat_id, user_id) VALUES (?, ?)";
	for (int uid : { user1ID, user2ID })
	{
		sqlite3_stmt* ms = nullptr;
		if (sqlite3_prepare_v2(db_, insertMember, -1, &ms, nullptr) != SQLITE_OK)
		{
			Logger::instance().error("prepare insert member failed: {}", sqlite3_errmsg(db_));
			executeSql("ROLLBACK", "rollback");
			return -1;
		}
		sqlite3_bind_int(ms, 1, chatID);
		sqlite3_bind_int(ms, 2, uid);
		if (sqlite3_step(ms) != SQLITE_DONE)
		{
			Logger::instance().error("insert member failed: {}", sqlite3_errmsg(db_));
			sqlite3_finalize(ms);
			executeSql("ROLLBACK", "rollback");
			return -1;
		}
		sqlite3_finalize(ms);
	}

	if (!executeSql("COMMIT", "commit"))
		return -1;

	return chatID;
}

std::vector<ChatListEntry> Database::getUserChats(int userID)
{
	std::vector<ChatListEntry> result;
	const char* sql =
		"SELECT c.id, u.username FROM chats c "
		"JOIN chat_members other ON other.chat_id = c.id AND other.user_id != ? "
		"JOIN users u ON u.id = other.user_id "
		"JOIN chat_members mine ON mine.chat_id = c.id AND mine.user_id = ? "
		"WHERE c.is_group = 0 "
		"ORDER BY c.id DESC";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		Logger::instance().error("prepare failed: {}", sqlite3_errmsg(db_));
		return result;
	}
	sqlite3_bind_int(stmt, 1, userID);
	sqlite3_bind_int(stmt, 2, userID);

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		ChatListEntry e;
		e.chatID       = static_cast<uint32_t>(sqlite3_column_int(stmt, 0));
		const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		e.peerUsername = name ? name : "";
		result.push_back(e);
	}
	sqlite3_finalize(stmt);
	return result;
}
