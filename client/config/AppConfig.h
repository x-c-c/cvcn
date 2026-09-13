#pragma once
#include <cstddef>
#include <cstdint>

/**
 * @file AppConfig.h
 * @brief Константы клиента: сетевые параметры, логирование, лимиты.
 *
 * Все «магические числа» и строки собраны в одном месте.
 * Меняется здесь — действует по всему клиенту.
 */
namespace config
{
    // --- Сеть ---
    inline constexpr const char* DEFAULT_SERVER_HOST = "127.0.0.1";
    inline constexpr uint16_t    DEFAULT_SERVER_PORT = 55550;

    // --- Логи ---
    inline constexpr const char* LOG_FILE_PATH      = "logs/client.log";
    inline constexpr const char* LOGGER_NAME        = "client_logger";
    inline constexpr const char* LOG_PATTERN        = "[%Y-%m-%d %H:%M:%S.%e] [%-8l] %v";
    inline constexpr std::size_t LOG_MAX_FILE_SIZE  = 5 * 1024 * 1024;
    inline constexpr std::size_t LOG_MAX_FILE_COUNT = 3;

    // --- UI ---
    inline constexpr const char* CHAT_TIME_FORMAT = "HH:mm";

    // --- Валидация ---
    inline constexpr std::size_t MAX_USERNAME_LENGTH = 64;
    inline constexpr std::size_t MAX_PASSWORD_LENGTH = 127;
    inline constexpr std::size_t MAX_MESSAGE_LENGTH  = 4096;
}
