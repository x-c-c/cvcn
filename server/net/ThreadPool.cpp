#include "ThreadPool.h"
#include "Logger.h"
#include <chrono>
#include <exception>

ThreadPool::ThreadPool(size_t numThreads)
{
    if (numThreads == 0)
        numThreads = 1;

    workers_.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i)
    {
        workers_.emplace_back([this]() { workerLoop(); });
    }

    LOG_INFO("ThreadPool started with {} workers", numThreads);
}

ThreadPool::~ThreadPool()
{
    stop();
}

void ThreadPool::submit(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_)
            return;
        tasks_.push_back(std::move(task));
    }
    cvNewTask_.notify_one();
}

void ThreadPool::waitIdle()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cvIdle_.wait(lock, [this]() {
        return tasks_.empty() && inFlight_ == 0;
    });
}

bool ThreadPool::hasPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return !tasks_.empty() || inFlight_ > 0;
}

void ThreadPool::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_)
            return;
        stopping_ = true;
    }
    cvNewTask_.notify_all();

    for (auto& worker : workers_)
    {
        if (worker.joinable())
            worker.join();
    }
    workers_.clear();
}

void ThreadPool::workerLoop()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cvNewTask_.wait(lock, [this]() {
                return stopping_ || !tasks_.empty();
            });

            if (stopping_ && tasks_.empty())
                return;

            task = std::move(tasks_.front());
            tasks_.pop_front();
            ++inFlight_;
        }

        try
        {
            task();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Unhandled exception in worker task: {}", e.what());
        }
        catch (...)
        {
            LOG_ERROR("Unknown exception in worker task");
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            --inFlight_;
            if (tasks_.empty() && inFlight_ == 0)
                cvIdle_.notify_all();
        }
    }
}
