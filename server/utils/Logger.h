/**
 * @file    Logger.h
 * @brief   Синглтон-логгер с двумя sink-ами: цветной консолью и
 *          ротируемым файлом.
 *
 * @details
 *
 *   Единственная точка логирования в проекте. Скрывает внутри
 *   spdlog и предоставляет простой интерфейс: instance().info(...),
 *   instance().error(...). Формат строки, уровни, пути — всё
 *   внутри.
 *
 *   Устроен как синглтон Майерса: статический локальный объект
 *   в instance(). Инициализация при первом вызове. Деструктор
 *   вызывается при выходе из программы, корректно закрывает файл.
 *
 *   Два sink-а:
 *     - stdout_color_sink — цветной вывод в консоль, уровень info
 *                           и выше (debug/trace не показываются);
 *     - rotating_file_sink — файл logs/server.log, уровень trace
 *                            и выше, ротация по 5 МБ, 3 файла.
 *
 *   Формат строки задаётся PATTERN:
 *     [%Y-%m-%d %H-%M-%S.%e] [%^%l%$] [thread %t] %v
 *     дата время.мс          уровень   поток       сообщение
 * @see     fmt::format, spdlog::logger
 */

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
    Logger();       ///< Создаёт sink-и и spdlog-логгер.
    ~Logger();      ///< Флашит и удаляет логгер из реестра spdlog.

    std::shared_ptr<spdlog::logger> logger_;    ///< Владеет. Один на процесс.

    static constexpr const char* LOG_FILE_PATH  = "logs/server.log";   ///< Путь к логу.
    static constexpr size_t MAX_FILE_SIZE       = 5 * 1024 * 1024;     ///< 5 МБ на файл.
    static constexpr size_t MAX_FILE_COUNT      = 3;                   ///< 3 файла ротации.
    static constexpr const char* LOGGER_NAME    = "server_logger";     ///< Имя в реестре spdlog.
    static constexpr const char* PATTERN        = "[%Y-%m-%d %H-%M-%S.%e] [%^%l%$] [thread %t] %v";

public:
    /**
     * @brief Возвращает единственный экземпляр логгера.
     *
     * @details
     *   Классический синглтон Майерса: статический локальный объект
     *   создаётся при первом вызове и разрушается при выходе из
     *   программы.
     */
    static Logger& instance();

    Logger(const Logger&) = delete;             ///< Синглтон — не копируется.
    Logger& operator=(const Logger&) = delete;

    //  Обёртки без форматирования 
    // Эти методы принимают готовую строку и передают её spdlog-у.
    // Используются внутри шаблонных перегрузок ниже и в редких
    // случаях, когда строка уже собрана.

    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);

    //  Шаблонные перегрузки с fmt-форматированием 
    // Принимают строку с {} и любое количество аргументов. Внутри
    // вызывают fmt::format и передают результат в обёртку выше.

    /**
     * @brief Логирует сообщение с подстановкой аргументов в {}.
     *
     * @tparam Args Типы аргументов (выводятся автоматически).
     * @param str   Строка формата с {} placeholder-ами.
     * @param args  Значения, подставляемые в {}.
     *
     * @details
     *   fmt::format подставляет args вместо {} в format и возвращает
     *   готовую строку. std::forward<Args>(args)... передаёт каждый
     *   аргумент без лишнего копирования. После этого вызывается
     *   обычный trace(const std::string&) для записи в лог.
     *
     *   Пример использования:
     *     Logger::instance().info("Порт {}, пользователь {}", 8080, "Alice");
     *   внутри превратится в:
     *     info(fmt::format("Порт {}, пользователь {}", 8080, "Alice"))
     *   результат:
     *     "Порт 8080, пользователь Alice"
     *
     * @warning fmt::format бросает fmt::format_error при неверной
     *          форматной строке (например, {} без соответствующего
     *          аргумента). Ошибка не ловится здесь — упадёт наружу.
     */
    template<typename... Args> void trace(const std::string& str, Args&&... args)
    {
        trace(fmt::format(str, std::forward<Args>(args)...));
    }

    /// @copydoc trace(const std::string&, Args&&...)
    template<typename... Args> void debug(const std::string& str, Args&&... args)
    {
        debug(fmt::format(str, std::forward<Args>(args)...));
    }

    /// @copydoc trace(const std::string&, Args&&...)
    template<typename... Args> void info(const std::string& str, Args&&... args)
    {
        info(fmt::format(str, std::forward<Args>(args)...));
    }

    /// @copydoc trace(const std::string&, Args&&...)
    template<typename... Args> void warn(const std::string& str, Args&&... args)
    {
        warn(fmt::format(str, std::forward<Args>(args)...));
    }

    /// @copydoc trace(const std::string&, Args&&...)
    template<typename... Args> void error(const std::string& str, Args&&... args)
    {
        error(fmt::format(str, std::forward<Args>(args)...));
    }

    /// @copydoc trace(const std::string&, Args&&...)
    template<typename... Args> void critical(const std::string& str, Args&&... args)
    {
        critical(fmt::format(str, std::forward<Args>(args)...));
    }
};
