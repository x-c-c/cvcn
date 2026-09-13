#include "Logger.h"
#include "AppConfig.h"
#include <iostream>

namespace {

std::shared_ptr<spdlog::logger> makeConsoleFallback()
{
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);

    auto logger = std::make_shared<spdlog::logger>("fallback_logger", consoleSink);
    logger->set_level(spdlog::level::trace);
    logger->set_pattern(config::LOG_PATTERN);
    return logger;
}

} // namespace

Logger::Logger()
{
    try
    {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(spdlog::level::info);

        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            config::LOG_FILE_PATH, config::LOG_MAX_FILE_SIZE, config::LOG_MAX_FILE_COUNT);
        fileSink->set_level(spdlog::level::trace);

        std::vector<spdlog::sink_ptr> sinks = {consoleSink, fileSink};
        logger_ = std::make_shared<spdlog::logger>(config::LOGGER_NAME,
                                                   sinks.begin(), sinks.end());
        logger_->set_level(spdlog::level::trace);
        logger_->set_pattern(config::LOG_PATTERN);
        logger_->flush_on(spdlog::level::info);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[logger] File sink unavailable (" << e.what()
                  << "), using console only" << std::endl;
        logger_ = makeConsoleFallback();
    }
    catch (...)
    {
        std::cerr << "[logger] Unknown error during init, using console only" << std::endl;
        logger_ = makeConsoleFallback();
    }
}

Logger::~Logger()
{
    if (logger_)
    {
        try
        {
            logger_->flush();
            spdlog::drop(logger_->name());
        }
        catch (...) { /* деструктор не должен бросать */ }
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

    try
    {
        const std::string component = componentFromFile(file);
        logger_->log(level, "[{:<18}] {}", component, message);
    }
    catch (...)
    {
        // молча: логировать о том, что логгер не работает, нельзя
    }
}
