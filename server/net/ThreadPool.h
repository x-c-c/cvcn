#pragma once
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <cstddef>

/**
 * @file ThreadPool.h
 * @brief Пул воркеров фиксированного размера.
 *
 * Задачи — std::function<void()>. Пул защищён мьютексом + condition_variable.
 *
 * Жизненный цикл:
 *   1. Конструктор создаёт N потоков.
 *   2. submit() кладёт задачу в очередь.
 *   3. waitIdle() возвращается, когда очередь пуста и нет задач в работе.
 *   4. stop() завершает потоки. Перед stop() имеет смысл вызвать waitIdle().
 */
class ThreadPool
{
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief Поставить задачу в очередь.
     * @param task задача без аргументов
     * @note Безопасно из любого потока. После stop() вызов игнорируется.
     */

    void submit(std::function<void()> task);

    /** @brief Ждать, пока все задачи (в очереди и в работе) не завершатся. */
    void waitIdle();

    /** @brief Есть ли незавершённые задачи. */
    bool hasPendingTasks();

    /** @brief Остановить потоки. После вызова submit нельзя. */
    void stop();

private:
    std::vector<std::thread> workers_;
    std::deque<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cvNewTask_;
    std::condition_variable cvIdle_;
    bool stopping_ = false;
    size_t inFlight_ = 0;

    void workerLoop();
};
