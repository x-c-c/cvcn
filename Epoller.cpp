/**
 * @file Epoller.cpp
 * @brief Реализация мультиплексирования через epoll.
 */

#include "Epoller.h"
#include "ClientSession.h"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

Epoller::Epoller(): epollFileDescriptor_(epoll_create1(0)), running_(true){}
Epoller::~Epoller()
{
	for (std::unordered_map<int, ClientSession*>::iterator it = sessions_.begin();
		 it != sessions_.end(); ++it)
	{
		ClientSession* session = it->second;
		if (!session->isClosed())
			session->closeSession();
		delete session;
	}
	sessions_.clear();
	close(epollFileDescriptor_);
}

void Epoller::addFdToEpoll(int fileDescriptor, uint32_t events)
{
	epoll_event event;
	event.data.fd = fileDescriptor;
	event.events = events;
	if (epoll_ctl(epollFileDescriptor_, EPOLL_CTL_ADD, fileDescriptor, &event) == -1)
		perror("[ERROR] epoll_ctl add");
}

void Epoller::removeFdFromEpoll(int fileDescriptor)
{
	epoll_ctl(epollFileDescriptor_, EPOLL_CTL_DEL, fileDescriptor, nullptr);
}

void Epoller::modifyFdEvents(int fileDescriptor, uint32_t events)
{
	epoll_event event;
	event.data.fd = fileDescriptor;
	event.events = events;
	if (epoll_ctl(epollFileDescriptor_, EPOLL_CTL_MOD, fileDescriptor, &event) == -1)
		perror("[ERROR] epoll_ctl mod");
}

void Epoller::handleNewConnection(int serverSocketDescriptor)
{
	int clientDescriptor = accept(serverSocketDescriptor, nullptr, nullptr);
	if (clientDescriptor > 0)
	{
		int flags = fcntl(clientDescriptor, F_GETFL, 0);
		if (flags == -1)
		{
			perror("[ERROR] fcntl getfl");
			close(clientDescriptor);
			return;
		}
		if (fcntl(clientDescriptor, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			perror("[ERROR] fcntl setfl");
			close(clientDescriptor);
			return;
		}
		addFdToEpoll(clientDescriptor, EPOLLIN | EPOLLET);
		sessions_[clientDescriptor] = new ClientSession(clientDescriptor, this);
		std::cout << "[Epoller] new client fd=" << clientDescriptor << std::endl;
	}
	else
	{
		perror("[ERROR] accept error");
	}
}

void Epoller::closeClient(int fileDescriptor)
{
	removeFdFromEpoll(fileDescriptor);
	std::unordered_map<int, ClientSession*>::iterator it = sessions_.find(fileDescriptor);
	if (it != sessions_.end())
	{
		ClientSession* session = it->second;
		if (!session->isClosed())
			session->closeSession();
		delete session;
		sessions_.erase(it);
	}
	else
	{
		close(fileDescriptor);
	}
}

void Epoller::startEpollLoop(int serverSocketDescriptor)
{
	int flags = fcntl(serverSocketDescriptor, F_GETFL, 0);
	if (flags != -1)
		fcntl(serverSocketDescriptor, F_SETFL, flags | O_NONBLOCK);
	addFdToEpoll(serverSocketDescriptor, EPOLLIN);

	epoll_event readyEvents[maxEvents];
	while (running_)
	{
		int eventCount = epoll_wait(epollFileDescriptor_, readyEvents, maxEvents, 1000);
		if (eventCount == -1)
		{
			if (errno == EINTR)
				continue;
			perror("[ERROR] epoll_wait");
			break;
		}
		for (int i = 0; i < eventCount; ++i)
		{
			int sockFd = readyEvents[i].data.fd;
			uint32_t events = readyEvents[i].events;

			if (events & (EPOLLERR | EPOLLHUP))
			{
				if (sockFd == serverSocketDescriptor)
				{
					std::cerr << "[Epoller] error on server socket, stopping" << std::endl;
					running_ = false;
					break;
				}
				else
				{
					closeClient(sockFd);
				}
				continue;
			}

			if (events & EPOLLIN)
			{
				if (sockFd == serverSocketDescriptor)
					handleNewConnection(sockFd);
				else
				{
					std::unordered_map<int, ClientSession*>::iterator it = sessions_.find(sockFd);
					if (it != sessions_.end())
						it->second->handleRead();
				}
			}

			if (events & EPOLLOUT)
			{
				std::unordered_map<int, ClientSession*>::iterator it = sessions_.find(sockFd);
				if (it != sessions_.end())
					it->second->handleWrite();
			}
		}
	}
}

void Epoller::stopEpollLoop()
{
	running_ = false;
}
