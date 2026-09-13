#include "ChatRepository.h"
#include "Logger.h"

ChatRepository::ChatRepository(sqlite3* db): db_(db){}

std::vector<std::string> ChatRepository::findUsers(const std::string& query, int excludeUserID)
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

	const std::string pattern = query + "%";
	sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt,  2, excludeUserID);

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		if (name)
			result.emplace_back(name);
	}
	sqlite3_finalize(stmt);
	return result;
}

int ChatRepository::findOrCreateDirectChat(int user1ID, int user2ID)
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

	// Создаём новый чат в транзакции
	char* errMsg = nullptr;
	if (sqlite3_exec(db_, "BEGIN", nullptr, nullptr, &errMsg) != SQLITE_OK)
	{
		Logger::instance().error("BEGIN failed: {}", errMsg ? errMsg : "unknown");
		if (errMsg) sqlite3_free(errMsg);
		return -1;
	}

	const char* insertChat = "INSERT INTO chats (is_group, name) VALUES (0, NULL)";
	if (sqlite3_exec(db_, insertChat, nullptr, nullptr, &errMsg) != SQLITE_OK)
	{
		Logger::instance().error("insert chat failed: {}", errMsg ? errMsg : "unknown");
		if (errMsg) sqlite3_free(errMsg);
		sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
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
			sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
			return -1;
		}
		sqlite3_bind_int(ms, 1, chatID);
		sqlite3_bind_int(ms, 2, uid);
		if (sqlite3_step(ms) != SQLITE_DONE)
		{
			Logger::instance().error("insert member failed: {}", sqlite3_errmsg(db_));
			sqlite3_finalize(ms);
			sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
			return -1;
		}
		sqlite3_finalize(ms);
	}

	if (sqlite3_exec(db_, "COMMIT", nullptr, nullptr, &errMsg) != SQLITE_OK)
	{
		Logger::instance().error("COMMIT failed: {}", errMsg ? errMsg : "unknown");
		if (errMsg) sqlite3_free(errMsg);
		sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
		return -1;
	}

	return chatID;
}

std::vector<ChatListEntry> ChatRepository::getUserChats(int userID)
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
		e.chatID = static_cast<uint32_t>(sqlite3_column_int(stmt, 0));
		const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		e.peerUsername = name ? name : "";
		result.push_back(e);
	}
	sqlite3_finalize(stmt);
	return result;
}

std::vector<int> ChatRepository::getChatMemberIDs(uint32_t chatID)
{
	std::vector<int> result;
	const char* sql = "SELECT user_id FROM chat_members WHERE chat_id = ?";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		Logger::instance().error("prepare failed: {}", sqlite3_errmsg(db_));
		return result;
	}
	sqlite3_bind_int(stmt, 1, static_cast<int>(chatID));

	while (sqlite3_step(stmt) == SQLITE_ROW)
		result.push_back(sqlite3_column_int(stmt, 0));
	sqlite3_finalize(stmt);
	return result;
}
