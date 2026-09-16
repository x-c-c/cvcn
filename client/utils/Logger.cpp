/**
 * @file    Logger.cpp
 * @brief   Реализация Logger: инициализация spdlog, обёртки уровней.
 *
 * @details
 *   Конструктор создаёт два sink-а и собирает из них spdlog::logger
 *   с общим уровнем trace. Уровень каждого sink-а задаётся отдельно:
 *   консоль — info+, файл — trace+. Это даёт разное «лицо» одному
 *   и тому же логу: в консоли только важное, в файле — всё.
 *
 *   logger_ регистрируется в глобальном реестре spdlog под именем
 *   LOGGER_NAME, чтобы spdlog::drop в деструкторе корректно его
 *   удалил.
 *
 *   Методы trace/debug/info/... — тонкие обёртки над
 *   spdlog::logger::log(level, message). Никакой логики, только
 *   маршрутизация вызова к нужному уровню.
 *
 * @see Logger.h
 */

#include "./Logger.h"

Logger::Logger()
{
    //  Sink 1: цветная консоль 
    // stdout_color_sink_mt — потокобезопасный (multi-threaded) sink
    // в stdout. Цвета зависят от уровня: info — зелёный, warn —
    // жёлтый, error — красный.
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::info);    // trace/debug в консоль не идут

    //  Sink 2: ротируемый файл 
    // rotating_file_sink_mt: пишет в LOG_FILE_PATH, при достижении
    // MAX_FILE_SIZE файл сдвигается в .1, .1 → .2, и т.д. Держит
    // до MAX_FILE_COUNT файлов (включая текущий), старые удаляются.
    // Итого максимум ~15 МБ на диске.
    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        LOG_FILE_PATH, MAX_FILE_SIZE, MAX_FILE_COUNT);
    fileSink->set_level(spdlog::level::trace);      // в файл — всё

    //  Сборка логгера 
    // spdlog::logger пишет во все переданные sink-и. Их фильтрация
    // по уровню работает независимо: общий уровень trace пропускает
    // всё, а конкретный sink решает, выводить или нет.
    std::vector<spdlog::sink_ptr> sinks = {consoleSink, fileSink};
    logger_ = std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end());

    logger_->set_level(spdlog::level::trace);   // общий уровень — пропускать всё

    // Формат строки: [дата время.мс] [уровень] [поток] сообщение.
    // %^...%$ — маркеры окрашивания, работают только в color-sink-е.
    logger_->set_pattern(PATTERN);

    // Автоматический flush при записи уровня info и выше. trace/debug
    // могут оставаться в буфере до следующего flush — это быстрее
    // для горячего пути. При падении процесса потеряются только
    // trace/debug, что обычно приемлемо.
    logger_->flush_on(spdlog::level::info);
}

Logger::~Logger()
{
    if (logger_)
    {
        // Досбрасываем буфер на диск перед разрушением.
        logger_->flush();

        // Удаляем логгер из глобального реестра spdlog по имени.
        // Без этого деструктор spdlog::logger может ругаться на
        // «already registered» при повторном создании с тем же именем.
        spdlog::drop(logger_->name());
    }
}

Logger& Logger::instance()
{
    // Синглтон Майерса: статический локальный объект. Создаётся
    // при первом вызове (потокобезопасно в C++11), разрушается
    // автоматически при выходе из программы.
    static Logger instance;
    return instance;
}

//  Обёртки уровней 
// Каждая передаёт вызов в spdlog::logger с соответствующим уровнем.
// Функции короткие, компилятор их заинлайнит.

void Logger::trace(const std::string& message)    { logger_->trace(message); }
void Logger::debug(const std::string& message)    { logger_->debug(message); }
void Logger::info(const std::string& message)     { logger_->info(message); }
void Logger::warn(const std::string& message)     { logger_->warn(message); }
void Logger::error(const std::string& message)    { logger_->error(message); }
void Logger::critical(const std::string& message) { logger_->critical(message); }
