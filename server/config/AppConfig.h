#pragma once
#include <cstddef>
#include <cstdint>

/**
 * @file AppConfig.h
 * @brief Константы сервера: сеть, БД, логи, лимиты, буферы.
 *
 * Все «магические числа» и строки собраны здесь.
 */
namespace config
{
    // --- Сеть ---
    inline constexpr const char* DEFAULT_DB_PATH = "chat.db";
    inline constexpr uint16_t    DEFAULT_PORT    = 55550;
    inline constexpr int         LISTEN_BACKLOG  = 128;

    // --- Логи ---
    inline constexpr const char* LOG_FILE_PATH      = "logs/server.log";
    inline constexpr const char* LOGGER_NAME        = "server_logger";
    inline constexpr const char* LOG_PATTERN        = "[%Y-%m-%d %H:%M:%S.%e] [%-8l] %v";
    inline constexpr std::size_t LOG_MAX_FILE_SIZE  = 5 * 1024 * 1024;
    inline constexpr std::size_t LOG_MAX_FILE_COUNT = 3;

    // --- Валидация ---
    inline constexpr std::size_t MAX_USERNAME_LENGTH = 64;
    inline constexpr std::size_t MAX_PASSWORD_LENGTH = 127;
    inline constexpr std::size_t MAX_MESSAGE_LENGTH  = 4096;

    // --- Буферы ---
    inline constexpr std::size_t SESSION_READ_BUFFER_SIZE    = 4096;
    inline constexpr std::size_t PACKET_ASSEMBLER_MAX_BUFFER = 64 * 1024;
    inline constexpr std::size_t MAX_REASONABLE_PACKET_BODY  = MAX_MESSAGE_LENGTH + 1024;

    // --- Пароли ---
    inline constexpr const char* PASSWORD_HASH_PREFIX = "hash_";
}
