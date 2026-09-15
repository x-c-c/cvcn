/**
 * @file    Database.cpp
 * @brief   Реализация Database: SQLite-обёртка для таблицы users.
 *
 * @details
 *   Все запросы готовятся через sqlite3_prepare_v2 + bind, а не через
 *   конкатенацию строк — это защита от SQL-инъекций и корректная
 *   работа с бинарными/UTF-8 данными.
 *
 *   Каждый публичный метод:
 *     1. Готовит stmt (prepare).
 *     2. Подставляет параметры (bind).
 *     3. Выполняет (step).
 *     4. Освобождает stmt (finalize).
 *     5. Проверяет код возврата и логирует ошибку.
 *
 *   Возврат stmt после step обязателен всегда: иначе утечка памяти
 *   в SQLite. Даже при ошибке step finalize нужен.
 *
 * @see Database.h
 */

#include "./Database.h"
#include "../utils/Logger.h"
#include <stdexcept>
#include <cstring>

Database::Database(const std::string& dbPath)
{
    // Если БД не открылась — сервер работать не сможет. Бросаем
    // std::runtime_error, чтобы main поймал и завершился с кодом 1.
    if (!open(dbPath))
        throw std::runtime_error("Failed to open database: " + dbPath);
}

Database::~Database()
{
    // sqlite3_close идемпотентен: nullptr тоже допустим.
    if (db_)
        sqlite3_close(db_);
}

bool Database::open(const std::string& dbPath)
{
    // sqlite3_open создаёт файл, если его нет, и открывает соединение.
    // WAL не включается автоматически — отдельный PRAGMA ниже.
    const int returnCode = sqlite3_open(dbPath.c_str(), &db_);
    if (returnCode != SQLITE_OK)
    {
        // sqlite3_errmsg работает с db_ даже при ошибке открытия —
        // если db_ != nullptr, там будет осмысленное сообщение.
        Logger::instance().error("Cannot open database: {}", sqlite3_errmsg(db_));
        return false;
    }

    // WAL (write-ahead log): транзакции пишутся в отдельный файл .wal,
    // читатели не блокируют писателя. Для учебного проекта не критично,
    // но безвредно и полезно, если появятся параллельные чтения.
    // Результат игнорируется — не включится WAL, будет работать
    // в обычном режиме.
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    // Схема таблицы users. IF NOT EXISTS делает запрос идемпотентным:
    // можно вызывать при каждом запуске сервера.
    //   - id           — суррогатный первичный ключ, AUTOINCREMENT;
    //   - username     — UNIQUE защищает от дубликатов на уровне БД;
    //   - password_hash— не пароль, а его представление;
    //   - created_at   — метка создания, автозаполняется SQLite.
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT NOT NULL UNIQUE,"
        "  password_hash TEXT NOT NULL,"
        "  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ");";

    // errMsg заполняется SQLite только при ошибке. Память под строку
    // выделяет библиотека — освобождать через sqlite3_free.
    char* errMsg = nullptr;
    const int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        Logger::instance().error("SQL error: {}", errMsg ? errMsg : "(no message)");
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::addUser(const std::string& username, const std::string& passwordHash)
{
    // Подготовленный запрос с двумя placeholder-ами. prepare разбирает
    // SQL один раз; bind подставляет значения без склейки строк —
    // защита от инъекций и корректная работа с кавычками/юникодом.
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    sqlite3_stmt* stmt = nullptr;

    // prepare может упасть (синтаксис, отсутствие таблицы). Проверка
    // критична: без неё stmt останется nullptr и bind упадёт с UB.
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        Logger::instance().error("prepare(addUser) failed: {}", sqlite3_errmsg(db_));
        return false;
    }

    // SQLITE_STATIC: библиотека не копирует строки, работает с нашими
    // c_str() напрямую. Это безопасно, потому что username и
    // passwordHash живут до конца функции (step выполнится внутри).
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_STATIC);

    // step выполняет INSERT. SQLITE_DONE — успех. SQLITE_CONSTRAINT
    // здесь означал бы нарушение UNIQUE, но вызывающий код уже
    // проверил isUserExist, так что это будет что-то другое.
    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);     // освобождаем stmt всегда

    if (rc != SQLITE_DONE)
    {
        Logger::instance().error("Failed to add user '{}': {}",
                                 username, sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

std::string Database::getUserPasswordHash(const std::string& username)
{
    const char* sql = "SELECT password_hash FROM users WHERE username = ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        Logger::instance().error("prepare(getUserPasswordHash) failed: {}",
                                 sqlite3_errmsg(db_));
        return {};
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    std::string hash;
    // SQLITE_ROW — есть результат. Функция ожидает не более одной
    // строки, потому что username UNIQUE. Если строк нет — остаётся
    // пустая строка, и вызывающий трактует это как «не найден».
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // sqlite3_column_text возвращает const unsigned char*.
        // reinterpret_cast нужен, чтобы привести к char* для std::string.
        // Указатель валиден до следующего step/finalize — поэтому
        // копируем в std::string немедленно.
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text)
            hash = reinterpret_cast<const char*>(text);
    }
    sqlite3_finalize(stmt);
    return hash;
}

bool Database::isUserExist(const std::string& username)
{
    // Раз пользователь есть тогда и только тогда, когда у него есть
    // хэш — проверять существование отдельным SELECT COUNT не нужно.
    // Минус: ошибка SQL и «нет пользователя» сливаются в одно false.
    return !getUserPasswordHash(username).empty();
}

int Database::getUserID(const std::string& username)
{
    const char* sql = "SELECT id FROM users WHERE username = ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        Logger::instance().error("prepare(getUserID) failed: {}", sqlite3_errmsg(db_));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    // -1 — сентинел «нет такого пользователя». id в SQLite начинается
    // с 1, так что -1 не пересекается с реальными значениями.
    int userID = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        userID = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return userID;
}
