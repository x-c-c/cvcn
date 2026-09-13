#include "Logger.h"

Logger::Logger()
{
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_level(spdlog::level::info);

	auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
		LOG_FILE_PATH, MAX_FILE_SIZE, MAX_FILE_COUNT);
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

namespace {
	std::string componentFromFile(const char* file)
	{
		std::string path = file;
		const auto slash = path.find_last_of("/\\");
		if (slash != std::string::npos)
			path = path.substr(slash + 1);

		const auto dot = path.find_last_of('.');
		if (dot != std::string::npos)
			path = path.substr(0, dot);

		return path;
	}
}

void Logger::log(const char* file,
				 spdlog::level::level_enum level,
				 const std::string& message)
{
	if (!logger_)
		return;

	const std::string component = componentFromFile(file);
	logger_->log(level, "[{:<18}] {}", component, message);
}
