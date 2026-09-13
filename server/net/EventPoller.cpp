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
	stopEventLoop();
}


void EventPoller::setNewConnectionCallback(NewConnectionCallback callback)
{
	onNewConnection_ = std::move(callback);
}
void EventPoller::setReadEventCallback(ReadEventCallback callback)
{
	onRead_ = std::move(callback);
}
void EventPoller::setWriteEventCallback(WriteEventCallback callback)
{
	onWrite_ = std::move(callback);
}
void EventPoller::setErrorEventCallback(ErrorEventCallback callback)
{
	onError_ = std::move(callback);
}


void EventPoller::addFileDescriptor(int fileDescriptor, uint32_t events)
{
	epoll_event event{};				// странно звучит - eventpoll_event event
	event.data.fd = fileDescriptor;
	event.events = events;
	if (epoll_ctl(epollFD_, EPOLL_CTL_ADD, fileDescriptor, &event) == -1)
	{
		LOG_ERROR("epoll_ctl ADD failed for fd {}: {}", fileDescriptor, strerror(errno));
	}
}

void EventPoller::removeFileDescriptor(int fileDescriptor)
{
	epoll_ctl(epollFD_, EPOLL_CTL_DEL, fileDescriptor, nullptr);
}

void EventPoller::modifyFileDescriptorEvents(int fileDescriptor, uint32_t events)
{
	epoll_event event{};
	event.data.fd = fileDescriptor;
	event.events = events;
	if (epoll_ctl(epollFD_, EPOLL_CTL_MOD, fileDescriptor, &event) == -1)
	{
		LOG_ERROR("epoll_ctl MOD failed for fd {}: {}", fileDescriptor, strerror(errno));
	}
}

void EventPoller::startEventLoop(int serverFileDescriptor)
{
	int flags = fcntl(serverFileDescriptor, F_GETFL, 0);
	if (flags == -1 || fcntl(serverFileDescriptor, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		LOG_CRITICAL("fcntl O_NONBLOCK on server fd failed: {}", strerror(errno));
		return;
	}
	addFileDescriptor(serverFileDescriptor, EPOLLIN);
	running_ = true;

	epoll_event readyEvents[MAX_EVENTS];
	while (running_.load() && !ShutdownSignal::isRequested())
	{
		int eventCount = epoll_wait(epollFD_, readyEvents, MAX_EVENTS, WAIT_MILLISECONDS);
		if (eventCount == -1)
		{
			if (errno == EINTR)
				continue;
			LOG_CRITICAL("epoll_wait failed: {}", strerror(errno));
			running_ = false;
			break;
		}

		for (int i = 0; i < eventCount; ++i)
		{
			int fileDescriptor = readyEvents[i].data.fd;
			uint32_t events = readyEvents[i].events;

			if (events & (EPOLLERR | EPOLLHUP))
			{
				if (fileDescriptor == serverFileDescriptor)
				{
					LOG_CRITICAL("Critical error on server socket (fd {}), stopping", serverFileDescriptor);
					running_ = false;
					break;
				}
				if (onError_)
					onError_(fileDescriptor, events);
				continue;
			}

			if (events & EPOLLIN)
			{
				if (fileDescriptor == serverFileDescriptor)
				{
					int clientFileDescriptor = accept(serverFileDescriptor, nullptr, nullptr);
					if (clientFileDescriptor < 0)
					{
						if (errno != EAGAIN && errno != EWOULDBLOCK)
							LOG_ERROR("accept() failed: {}", strerror(errno));
					}
					else
					{
						int cflags = fcntl(clientFileDescriptor, F_GETFL, 0);
						if (cflags == -1 || fcntl(clientFileDescriptor, F_SETFL, cflags | O_NONBLOCK) == -1)
						{
							LOG_ERROR("fcntl O_NONBLOCK on client fd {} failed: {}", clientFileDescriptor, strerror(errno));
							close(clientFileDescriptor);
						}
						else
						{
							addFileDescriptor(clientFileDescriptor, EPOLLIN | EPOLLET);
							if (onNewConnection_)
								onNewConnection_(clientFileDescriptor);
						}
					}
				}
				else if (onRead_)
					onRead_(fileDescriptor);
			}

			if (events & EPOLLOUT)
			{
				if (onWrite_)
					onWrite_(fileDescriptor);
			}
		}
	}
}

void EventPoller::stopEventLoop()
{
	running_ = false;
	if (epollFD_ != -1)
	{
		close(epollFD_);
		epollFD_ = -1;
	}
}
