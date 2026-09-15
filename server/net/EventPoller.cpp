/**
 * @file    EventPoller.cpp
 * @brief   Реализация EventPoller.
 *
 * @details
 *   startEpollLoop() крутит epoll_wait с таймаутом 1 сек, что даёт
 *   шанс проверить ShutdownSignal::isStopRequested() даже если
 *   события не приходят. Для каждого готового fd определяется набор
 *   событий и вызывается соответствующий callback.
 *
 *   Клиентские fd регистрируются в EPOLLIN | EPOLLET. Это значит,
 *   что callback onRead_ вызывается один раз при появлении данных,
 *   и внутри ClientSession::handleRead нужно вычитать всё до EAGAIN —
 *   иначе новые байты не разбудят epoll.
 *
 * @see EventPoller.h
 */

#include "./EventPoller.h"
#include "../utils/Logger.h"
#include "../app/ShutdownSignal.h"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>

EventPoller::EventPoller(): epollFD_(epoll_create1(0)), running_(false){}

EventPoller::~EventPoller()
{
    stopEpollLoop();
}

void EventPoller::setNewConnectionCallback(newConnectionCallback cb)
{
    onNewConnection_ = std::move(cb);
}

void EventPoller::setReadEventCallback(readEventCallback cb)
{
    onRead_ = std::move(cb);
}

void EventPoller::setWriteEventCallback(writeEventCallback cb)
{
    onWrite_ = std::move(cb);
}

void EventPoller::setErrorEventCallback(errorEventCallback cb)
{
    onError_ = std::move(cb);
}

void EventPoller::addFdToEpoll(int fileDescriptor, uint32_t events)
{
    epoll_event event{};
    event.data.fd = fileDescriptor;
    event.events = events;

    if (epoll_ctl(epollFD_, EPOLL_CTL_ADD, fileDescriptor, &event) == -1)
    {
        Logger::instance().error("epoll_ctl ADD failed for fd {}: {}", fileDescriptor, strerror(errno));
    }
}

void EventPoller::removeFdFromEpoll(int fileDescriptor)
{
    epoll_ctl(epollFD_, EPOLL_CTL_DEL, fileDescriptor, nullptr);
}

void EventPoller::modifyFdEvents(int fileDescriptor, uint32_t events)
{
    epoll_event event{};
    event.data.fd = fileDescriptor;
    event.events = events;

    if (epoll_ctl(epollFD_, EPOLL_CTL_MOD, fileDescriptor, &event) == -1)
    {
        Logger::instance().error("epoll_ctl MOD failed for fd {}: {}",
                                 fileDescriptor, strerror(errno));
    }
}

void EventPoller::startEpollLoop(int serverSocketFD)
{
    int flags = fcntl(serverSocketFD, F_GETFL, 0);
    if (flags != -1)
    {
        fcntl(serverSocketFD, F_SETFL, flags | O_NONBLOCK);
    }

    addFdToEpoll(serverSocketFD, EPOLLIN);
    running_ = true;

    epoll_event readyEvents[MAX_EVENTS];

    while (running_.load() && !ShutdownSignal::isStopRequested())
    {
        // Таймаут 1000 мс: если событий нет, epoll_wait вернёт 0,
        // и цикл проверит isStopRequested() снова. Без таймаута
        // Ctrl+C мог бы «зависнуть» до следующего события.
        const int eventCount = epoll_wait(epollFD_, readyEvents, MAX_EVENTS, WAIT_MILLISECONDS);

        if (eventCount == -1)
        {
            if (errno == EINTR)
                continue;   // прервано сигналом — не ошибка, продолжаем

            Logger::instance().critical("epoll_wait failed: {}", strerror(errno));
            break;
        }

        for (int i = 0; i < eventCount; ++i)
        {
            const int socketFD   = readyEvents[i].data.fd;
            const uint32_t events = readyEvents[i].events;
            
            if (events & (EPOLLERR | EPOLLHUP))
            {
                if (socketFD == serverSocketFD)
                {
                    // Ошибка на слушающем сокете означает, что принимать
                    // соединения больше нельзя. Останавливаем сервер.
                    Logger::instance().critical("Critical error on server socket (fd {}), stopping",serverSocketFD);
                    running_ = false;
                    break;
                }

                if (onError_)
                    onError_(socketFD, events);
                continue;
            }

            // Данные для чтения 
            if (events & EPOLLIN)
            {
                if (socketFD == serverSocketFD)
                {
                    const int clientSocketFD = accept(serverSocketFD, nullptr, nullptr);
                    if (clientSocketFD < 0)
                    {
                        // EAGAIN/EWOULDBLOCK — очередь пуста, это норма.
                        if (errno != EAGAIN && errno != EWOULDBLOCK)
                            Logger::instance().error("accept() failed: {}", strerror(errno));
                    }
                    else
                    {
                        // Переводим клиентский сокет в неблокирующий режим.
                        const int cflags = fcntl(clientSocketFD, F_GETFL, 0);
                        if (cflags == -1 ||
                            fcntl(clientSocketFD, F_SETFL, cflags | O_NONBLOCK) == -1)
                        {
                            Logger::instance().error("fcntl O_NONBLOCK on client fd {} failed: {}", clientSocketFD, strerror(errno));
                            close(clientSocketFD);
                        }
                        else
                        {
                            addFdToEpoll(clientSocketFD, EPOLLIN | EPOLLET);
                            if (onNewConnection_)
                                onNewConnection_(clientSocketFD);
                        }
                    }
                }
                else if (onRead_)
                {
                    onRead_(socketFD);
                }
            }

            // Сокет готов к записи
            if (events & EPOLLOUT)
            {
                if (onWrite_)
                    onWrite_(socketFD);
            }
        }
    }
}

void EventPoller::stopEpollLoop()
{
    running_ = false;

    if (epollFD_ != -1)
    {
        close(epollFD_);
        epollFD_ = -1;
    }
}
