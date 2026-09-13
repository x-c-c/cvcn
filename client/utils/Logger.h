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
 * @brief Единая точка логирования с автоматическим указанием компонента.
 *
 * Формат вывода:
 *   [YYYY-MM-DD HH:MM:SS.mmm] [level    ] [Component        ] message
 *
 * Компонент берётся из имени файла, откуда вызван макрос.
 *
 * Все параметры (пути, имя логгера, размеры ротации) читаются из
 * config/AppConfig.h, в этом файле не дублируются.
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
        log(file, level, fmt::format(format, std::forward<Args>(args)...));
    }
};

#define LOG_TRACE(...)    Logger::instance().log(__FILE__, spdlog::level::trace,    __VA_ARGS__)
#define LOG_DEBUG(...)    Logger::instance().log(__FILE__, spdlog::level::debug,    __VA_ARGS__)
#define LOG_INFO(...)     Logger::instance().log(__FILE__, spdlog::level::info,     __VA_ARGS__)
#define LOG_WARN(...)     Logger::instance().log(__FILE__, spdlog::level::warn,     __VA_ARGS__)
#define LOG_ERROR(...)    Logger::instance().log(__FILE__, spdlog::level::err,      __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::instance().log(__FILE__, spdlog::level::critical, __VA_ARGS__)
