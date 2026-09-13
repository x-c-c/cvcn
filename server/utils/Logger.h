#pragma once
#include <string>
#include <memory>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <fmt/format.h>

namespace spdlog { class logger; }

/**
 * @file Logger.h
 * @brief Единая точка логирования с автокомпонентом и fallback на stderr.
 *
 * Формат: [YYYY-MM-DD HH:MM:SS.mmm] [level    ] [Component        ] message
 * Компонент — имя файла, откуда вызван макрос.
 *
 * Параметры читаются из config/AppConfig.h. Инициализация не бросает
 * исключений: если spdlog не смог открыть файл, логгер работает только
 * в консоль.
 */
class Logger
{
private:
    Logger();
    ~Logger();

    std::shared_ptr<spdlog::logger> logger_;

public:
    static Logger& instance();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(const char* file,
             spdlog::level::level_enum level,
             const std::string& message);

    template<typename... Args>
    void log(const char* file, spdlog::level::level_enum level,
             const std::string& format, Args&&... args)
    {
        std::string message;
        try
        {
            message = fmt::format(format, std::forward<Args>(args)...);
        }
        catch (const std::exception& e)
        {
            message = std::string("Log formatting error: ") + e.what();
        }
        log(file, level, message);
    }
};

#define LOG_TRACE(...)    Logger::instance().log(__FILE__, spdlog::level::trace,    __VA_ARGS__)
#define LOG_DEBUG(...)    Logger::instance().log(__FILE__, spdlog::level::debug,    __VA_ARGS__)
#define LOG_INFO(...)     Logger::instance().log(__FILE__, spdlog::level::info,     __VA_ARGS__)
#define LOG_WARN(...)     Logger::instance().log(__FILE__, spdlog::level::warn,     __VA_ARGS__)
#define LOG_ERROR(...)    Logger::instance().log(__FILE__, spdlog::level::err,      __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::instance().log(__FILE__, spdlog::level::critical, __VA_ARGS__)
