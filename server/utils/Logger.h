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
 * Компонент берётся из имени файла, откуда вызван макрос:
 *   LOG_INFO("msg") из AuthService.cpp -> [AuthService] msg
 *   LOG_WARN("msg") из main.cpp        -> [main] msg
 *
 * Используйте макросы LOG_*, не вызывайте Logger::instance().log() напрямую.
 */
class Logger
{
private:
	Logger();
	~Logger();

	std::shared_ptr<spdlog::logger> logger_;

	static constexpr const char* LOG_FILE_PATH  = "logs/server.log";
	static constexpr size_t      MAX_FILE_SIZE  = 5 * 1024 * 1024;
	static constexpr size_t      MAX_FILE_COUNT = 3;
	static constexpr const char* LOGGER_NAME    = "server_logger";
	static constexpr const char* PATTERN        =
		"[%Y-%m-%d %H:%M:%S.%e] [%-8l] %v";

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
