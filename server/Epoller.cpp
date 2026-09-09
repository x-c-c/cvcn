#include "Epoller.h"
#include "ClientSession.h"
#include "Database.h"
#include "Logger.h"
#include "SigintHandler.h"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

Epoller::Epoller(Database* db): epollFD_(epoll_create1(0)), running_(true), db_(db){}

Epoller::~Epoller()
{
	stopEpollLoop();
}

void Epoller::addFdToEpoll(int fileDescriptor, uint32_t events)
{
	epoll_event event;				// странно звучит - eventpoll_event event
	event.data.fd = fileDescriptor;
	event.events = events;
	if (epoll_ctl(epollFD_, EPOLL_CTL_ADD, fileDescriptor, &event) == -1)
	{
		Logger::instance().error("epoll_ctl ADD failed for fd {}: {}", fileDescriptor, strerror(errno));
	}
}

void Epoller::removeFdFromEpoll(int fileDescriptor)
{
	epoll_ctl(epollFD_, EPOLL_CTL_DEL, fileDescriptor, nullptr);
}

void Epoller::modifyFdEvents(int fileDescriptor, uint32_t events)
{
	epoll_event event;
	event.data.fd = fileDescriptor;
	event.events = events;
	if (epoll_ctl(epollFD_, EPOLL_CTL_MOD, fileDescriptor, &event) == -1)
	{
		Logger::instance().error("epoll_ctl MOD failed for fd {}: {}", fileDescriptor, strerror(errno));
	}
}

void Epoller::handleNewConnection(int serverSocketFD)
{
	int clientSocketFD = accept(serverSocketFD, nullptr, nullptr);
	if (clientSocketFD > 0)
	{
		int flags = fcntl(clientSocketFD, F_GETFL, 0);
		if (flags == -1)
		{
			Logger::instance().error("fcntl F_GETFL failed for fd {}: {}", clientSocketFD, strerror(errno));
			close(clientSocketFD);
			return;
		}
		if (fcntl(clientSocketFD, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			Logger::instance().error("fcntl F_SETFL O_NONBLOCK failed for fd {}: {}", clientSocketFD, strerror(errno));
			close(clientSocketFD);
			return;
		}
		addFdToEpoll(clientSocketFD, EPOLLIN | EPOLLET);
		sessions_[clientSocketFD] = new ClientSession(clientSocketFD, this, db_);
		Logger::instance().info("New client connected, fd={}", clientSocketFD);
	}
	else
	{
		Logger::instance().error("accept() failed: {}", strerror(errno));
	}
}

void Epoller::closeClient(int fileDescriptor)
{
	removeFdFromEpoll(fileDescriptor);
	auto it = sessions_.find(fileDescriptor);
	if (it != sessions_.end())
	{
		if (!it->second->isClosed())
		{
			it->second->closeSession();
		}
		delete it->second;
		sessions_.erase(it);
	}
	else
	{
		close(fileDescriptor);
	}
}

void Epoller::startEpollLoop(int serverSocketFD)
{
	int flags = fcntl(serverSocketFD, F_GETFL, 0);
	if (flags != -1)
	{
		fcntl(serverSocketFD, F_SETFL, flags | O_NONBLOCK);
	}
	addFdToEpoll(serverSocketFD, EPOLLIN);

	epoll_event readyEvents[MAX_EVENTS];
	while (running_.load() && !SigintHandler::isStopRequested())
	{
		int eventCount = epoll_wait(epollFD_, readyEvents, MAX_EVENTS, WAIT_MILLISECONDS);
		if (eventCount == -1)
		{
			if (errno == EINTR)
				continue;
			Logger::instance().critical("epoll_wait failed: {}", strerror(errno));
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
				else
				{
					closeClient(socketFD);
				}
				continue;
			}

			if (events & EPOLLIN)
			{
				if (socketFD == serverSocketFD)
				{
					handleNewConnection(socketFD);
				}
				else
				{
					auto it = sessions_.find(socketFD);
					if (it != sessions_.end())
					{
						it->second->handleRead();
					}
				}
			}

			if (events & EPOLLOUT)
			{
				auto it = sessions_.find(socketFD);
				if (it != sessions_.end())
				{
					it->second->handleWrite();
				}
			}
		}
	}
}

void Epoller::stopEpollLoop()
{
		for (auto it = sessions_.begin(); it != sessions_.end(); ++it)
	{
		if (!it->second->isClosed())
		{
			it->second->closeSession();
		}
		delete it->second;
	}
	sessions_.clear();
	close(epollFD_);
	running_ = false;
}
