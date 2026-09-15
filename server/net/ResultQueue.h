#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include <cstdint>

/**
 * @file ResultQueue.h
 * @brief Thread-safe очередь команд для main-thread.
 *
 * Воркеры не вызывают методы сессии напрямую: они кладут команду
 * в эту очередь и пишут 8 байт в eventfd. Main-thread, увидев
 * событие на eventfd, забирает все команды и выполняет их.
 *
 * Категории команд:
 *   SendRaw   — отправить байты клиенту по fd.
 *   Close     — закрыть сессию по fd.
 *   TaskDone  — воркер завершил задачу по fd (уменьшить in-flight).
 */
enum class CommandType
{
    SendRaw,
    Close,
    TaskDone
};

/**
 * @brief Одна команда из воркера в main-thread.
 */

struct SessionCommand
{
    CommandType type;
    int fd;
    std::vector<uint8_t> data;   // для SendRaw
};

/**
 * @brief Thread-safe очередь команд из воркеров в main-thread.
 *
 * Воркеры не вызывают методы сессии напрямую: они кладут SessionCommand
 * в эту очередь и пишут 8 байт в eventfd. Main-thread, увидев событие
 * на eventfd, вызывает drain() и выполняет все накопленные команды.
 *
 * @par Категории команд:
 *   - SendRaw  — отправить байты клиенту по fd;
 *   - Close    — закрыть сессию по fd;
 *   - TaskDone — воркер закончил задачу по fd (уменьшить in-flight).
 *
 * @note push() безопасен из любого потока, drain() — только main-thread.
 */

class ResultQueue
{
public:
    ResultQueue();
    ~ResultQueue();

    ResultQueue(const ResultQueue&) = delete;
    ResultQueue& operator=(const ResultQueue&) = delete;

    /** @brief fd eventfd, регистрируется в epoll для EPOLLIN. */
    int eventFd() const { return eventFd_; }

    /** @brief Положить команду и уведомить main-thread. */
    void push(SessionCommand cmd);

    /** @brief Забрать все накопленные команды. Вызывается только из main-thread. */
    std::deque<SessionCommand> drain();

private:
    int eventFd_ = -1;
    std::mutex mutex_;
    std::deque<SessionCommand> queue_;
};
