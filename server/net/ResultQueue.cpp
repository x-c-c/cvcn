#include "ResultQueue.h"
#include "Logger.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

ResultQueue::ResultQueue()
{
    eventFd_ = eventfd(0, EFD_NONBLOCK);
    if (eventFd_ == -1)
    {
        LOG_CRITICAL("eventfd() failed: {}", strerror(errno));
    }
}

ResultQueue::~ResultQueue()
{
    if (eventFd_ != -1)
        close(eventFd_);
}

void ResultQueue::push(SessionCommand cmd)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(cmd));
    }

    if (eventFd_ == -1)
        return;

    const uint64_t one = 1;
    ssize_t written = write(eventFd_, &one, sizeof(one));
    if (written == -1 && errno != EAGAIN)
    {
        LOG_ERROR("eventfd write failed: {}", strerror(errno));
    }
}

std::deque<SessionCommand> ResultQueue::drain()
{
    std::deque<SessionCommand> out;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        out.swap(queue_);
    }

    if (eventFd_ != -1)
    {
        uint64_t value;
        while (read(eventFd_, &value, sizeof(value)) > 0)
        {
            // читаем до опустошения, eventfd склеивает все записи в счётчик
        }
    }

    return out;
}
