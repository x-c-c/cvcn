/**
 * @file    Database.h
 * @brief   Владелец соединения с SQLite, доступ к таблице users.
 *
 * @details
 *
 *   Открывает файл chat.db в конструкторе, создаёт схему
 *   (CREATE TABLE IF NOT EXISTS), закрывает соединение в деструкторе.
 *   Методы возвращают bool/int/строку; при ошибке логируют через
 *   Logger и возвращают «пустое» значение (false, -1, "").
 *
 *   SQL-схема:
 *     users(id INTEGER PRIMARY KEY AUTOINCREMENT,
 *           username TEXT NOT NULL UNIQUE,
 *           password_hash TEXT NOT NULL,
 *           created_at TEXT DEFAULT CURRENT_TIMESTAMP)
 *
 *   Поле password_hash хранит не пароль, а его представление (сейчас —
 *   строка "hash_" + пароль, в будущем — настоящий хэш bcrypt/argon2).
 *
 * @see     ClientSession, main.cpp
 */

#pragma once
#include <string>
#include <sqlite3.h>

class Database
{
private:
    sqlite3* db_ = nullptr;     ///< Владеет. Закрывается в деструкторе.

    /**
     * @brief Открывает файл БД и создаёт схему.
     *
     * @param dbPath Путь к файлу (относительный или абсолютный).
     * @return true  — соединение открыто, схема готова;
     *         false — sqlite3_open или CREATE TABLE упали.
     *
     * @details
     *   Вызывается только из конструктора. Включает WAL-режим
     *   (журналирование через отдельный файл — быстрее и безопаснее
     *   при параллельных чтениях). Создаёт таблицу users, если её нет.
     *
     * @note При ошибке пишет в лог через sqlite3_errmsg, детали
     *       возвращает только через bool.
     */
    bool open(const std::string& dbPath);

public:
    /**
     * @brief Открывает БД по указанному пути.
     *
     * @param dbPath Путь к файлу chat.db.
     * @throws std::runtime_error Если open() вернул false.
     *
     * @note Путь относительный — резолвится от текущего рабочего
     *       каталога процесса. Обычно запускают сервер из его
     *       каталога, поэтому "chat.db" указывает на server/chat.db.
     */
    explicit Database(const std::string& dbPath);

    /**
     * @brief Закрывает соединение с БД.
     */
    ~Database();

    //  Пользователи
    /**
     * @brief Проверяет, зарегистрирован ли пользователь.
     *
     * @param username Имя пользователя.
     * @return true — запись есть; false — нет или запрос упал.
     *
     * @note Реализовано через getUserPasswordHash: если строка пустая,
     *       пользователя нет. Отдельный SELECT COUNT не нужен.
     */
    bool isUserExist(const std::string& username);

    /**
     * @brief Добавляет нового пользователя.
     *
     * @param username     Имя (UNIQUE в схеме — повторный INSERT упадёт).
     * @param passwordHash Хэш пароля (сейчас — строка "hash_" + пароль).
     * @return true — запись добавлена; false — SQL error.
     *
     * @note Дубликат username ловится через SQLITE_CONSTRAINT
     *       (UNIQUE) — метод вернёт false, лог запишет причину.
     *       Проверять isUserExist перед вызовом — на усмотрение
     *       вызывающего (сейчас ClientSession так и делает).
     */
    bool addUser(const std::string& username, const std::string& passwordHash);

    /**
     * @brief Возвращает сохранённый хэш пароля.
     *
     * @param username Имя пользователя.
     * @return Строка хэша или пустая строка, если пользователя нет
     *         или произошла ошибка.
     *
     * @note Пустая строка — это и «нет пользователя», и «ошибка SQL».
     *       Отличить их снаружи нельзя. Если понадобится — вернуть
     *       std::optional<std::string>.
     */
    std::string getUserPasswordHash(const std::string& username);

    /**
     * @brief Возвращает id пользователя.
     *
     * @param username Имя пользователя.
     * @return id из таблицы users или -1, если пользователь не найден
     *         или запрос упал.
     *
     * @note Пока не используется в проекте, но пригодится, когда
     *       появится рассылка сообщений: сервер будет подставлять
     *       senderID из сессии, а не из клиентского пакета.
     */
    int getUserID(const std::string& username);

    // Чаты
    // int createChat(bool isGroup, const std::string& name);
};
