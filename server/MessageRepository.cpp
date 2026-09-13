#include "MessageRepository.h"
#include "Logger.h"

MessageRepository::MessageRepository(sqlite3* db): db_(db){}

bool MessageRepository::saveMessage(uint32_t chatID, int senderID, const std::string& text)
{
	const char* sql = "INSERT INTO messages (chat_id, sender_id, text) VALUES (?, ?, ?)";
	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		Logger::instance().error("prepare failed: {}", sqlite3_errmsg(db_));
		return false;
	}
	sqlite3_bind_int(stmt,  1, static_cast<int>(chatID));
	sqlite3_bind_int(stmt,  2, senderID);
	sqlite3_bind_text(stmt, 3, text.c_str(), -1, SQLITE_TRANSIENT);

	const int rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (rc != SQLITE_DONE)
	{
		Logger::instance().error("Failed to save message (chat {}): {}", chatID, sqlite3_errmsg(db_));
		return false;
	}
	return true;
}
