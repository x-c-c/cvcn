#include "Logger.h"

Logger::Logger()
{
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_level(spdlog::level::info);
	
	auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(LOG_FILE_PATH, MAX_FILE_SIZE, MAX_FILE_COUNT);
	fileSink->set_level(spdlog::level::trace);
	
	std::vector<spdlog::sink_ptr> sinks = {consoleSink, fileSink};
	logger_ = std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end());
	logger_->set_level(spdlog::level::trace);
	logger_->set_pattern(PATTERN);
	logger_->flush_on(spdlog::level::info);
}

Logger::~Logger()
{
	if (logger_)
	{
		logger_->flush();
		spdlog::drop(logger_->name());
	}
}

Logger& Logger::instance()
{
	static Logger instance;
	return instance;
}

void Logger::trace(const std::string& message)	{ logger_->trace(message); }
void Logger::debug(const std::string& message)	{ logger_->debug(message); }
void Logger::info(const std::string& message)	{ logger_->info(message); }
void Logger::warn(const std::string& message)	{ logger_->warn(message); }
void Logger::error(const std::string& message)	{ logger_->error(message); }
void Logger::critical(const std::string& message){ logger_->critical(message); }
