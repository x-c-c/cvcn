#include "EventPoller.h"
#include "Logger.h"
#include "ShutdownSignal.h"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
EventPoller::EventPoller():
	epollFD_(epoll_create1(0)), running_(false){}

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
	epoll_event event{};				// странно звучит - eventpoll_event event
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
		Logger::instance().error("epoll_ctl MOD failed for fd {}: {}", fileDescriptor, strerror(errno));
	}
}

void EventPoller::startEpollLoop(int serverSocketFD)
{
	int flags = fcntl(serverSocketFD, F_GETFL, 0);
	if (flags == -1 || fcntl(serverSocketFD, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		Logger::instance().critical("fcntl O_NONBLOCK on server fd failed: {}", strerror(errno));
		return;
	}
	addFdToEpoll(serverSocketFD, EPOLLIN);
	running_ = true;

	epoll_event readyEvents[MAX_EVENTS];
	while (running_.load() && !ShutdownSignal::isRequested())
	{
		int eventCount = epoll_wait(epollFD_, readyEvents, MAX_EVENTS, WAIT_MILLISECONDS);
		if (eventCount == -1)
		{
			if (errno == EINTR)
				continue;
			Logger::instance().critical("epoll_wait failed: {}", strerror(errno));
			running_ = false;
			break;
		}

		for (int i = 0; i < eventCount; ++i)
		{
			int socketFD = readyEvents[i].data.fd;
			uint32_t events = readyEvents[i].events;

			if (events & (EPOLLERR | EPOLLHUP))
			{
				if (socketFD == serverSocketFD)
				{
					Logger::instance().critical("Critical error on server socket (fd {}), stopping", serverSocketFD);
					running_ = false;
					break;
				}
				if (onError_)
					onError_(socketFD, events);
				continue;
			}

			if (events & EPOLLIN)
			{
				if (socketFD == serverSocketFD)
				{
					int clientSocketFD = accept(serverSocketFD, nullptr, nullptr);
					if (clientSocketFD < 0)
					{
						if (errno != EAGAIN && errno != EWOULDBLOCK)
							Logger::instance().error("accept() failed: {}", strerror(errno));
					}
					else
					{
						int cflags = fcntl(clientSocketFD, F_GETFL, 0);
						if (cflags == -1 || fcntl(clientSocketFD, F_SETFL, cflags | O_NONBLOCK) == -1)
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
					onRead_(socketFD);
			}

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
