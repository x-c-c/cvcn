#pragma once
#include <string>
#include <memory>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <fmt/format.h>
namespace spdlog { class logger; }
class Logger
{
private:
	Logger();
	~Logger();
	std::shared_ptr<spdlog::logger> logger_;
	static constexpr const char* LOG_FILE_PATH	= "logs/server.log";
	static constexpr size_t MAX_FILE_SIZE		= 5 * 1024 * 1024;
	static constexpr size_t MAX_FILE_COUNT		= 3;
	static constexpr const char* LOGGER_NAME	= "server_logger";
	static constexpr const char* PATTERN		= "[%Y-%m-%d %H-%M-%S.%e] [%^%1%$] [thread %t] %v";
	
public:
	static Logger& instance();
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	
	void trace(const std::string& message);
	void debug(const std::string& message);
	void info(const std::string& message);
	void warn(const std::string& message);
	void error(const std::string& message);
	void critical(const std::string& message);
	
	// Принимает строку с {} и любое количество аргументов любых типов
	template<typename... Args> void trace(const std::string& str, Args&&... args)
	{
		/* 
		 * fmt::format подставляет args вместо {} в format и возвращает готовую строку.
		 * std::forward<Args>(args)... передаёт каждый аргумент без лишнего копирования.
		 * После этого вызывается обычный trace(const std::std::string&) для записи в лог.
		
						Пример использования
						
		 * Logger::instance().info("Порт {}, пользователь {}", 8080, "Alice");
		 * внутри превратится в: info(fmt::format("Порт {}, пользователь {}", 8080, "Alice"))
		 * результат: "Порт 8080, пользователь Alice" – и запись в лог.
		 */
		trace(fmt::format(str, std::forward<Args>(args)...));
	}
	template<typename... Args> void debug(const std::string& str, Args&&... args)
	{
		debug(fmt::format(str, std::forward<Args>(args)...));
	}
	template<typename... Args> void info(const std::string& str, Args&&... args)
	{
		info(fmt::format(str, std::forward<Args>(args)...));
	}
	template<typename... Args> void warn(const std::string& str, Args&&... args)
	{
		warn(fmt::format(str, std::forward<Args>(args)...));
	}
	template<typename... Args> void error(const std::string& str, Args&&... args)
	{
		error(fmt::format(str, std::forward<Args>(args)...));
	}
	template<typename... Args> void critical(const std::string& str, Args&&... args)
	{
		critical(fmt::format(str, std::forward<Args>(args)...));
	}
	
};
