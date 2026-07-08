#include "Epoller.h"
Epoller::Epoller(): epollFd_(epoll_create1(0)), running_(true){}
Epoller::~Epoller()
{
	close(epollFd_);
}

void Epoller::addFdToEpoll(int fd, uint32_t events)
{
	epoll_event	ev;
	ev.data.fd	= fd;
	ev.events	= events;
	epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
}

void Epoller::removeFdFromEpoll(int fd)
{
	epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
}

void Epoller::handleNewConnection(int serverFd)
{
	int clientFd = accept(serverFd, nullptr, nullptr);
	if (clientFd > 0)	// установить неблокирующий режим
	{
		int flags = fcntl(clientFd, F_GETFL, 0);
		fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
		addFdToEpoll(clientFd, EPOLLIN);
	}
	else
	{
		perror("accept error");
	}
}

void Epoller::handleClientData(int clientFd)
{
	char buffer[1024]{};
	ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
if (bytesRead > 0)
	{
	// обработка данных
	}
else
	{
		closeConnection(clientFd);
	}
}

void Epoller::closeConnection(int fd)
{
	removeFdFromEpoll(fd);
	close(fd);
}

void Epoller::startEpollLoop(int serverSocketFd)
{
	// сделать серверный сокет неблокирующим
	int flags = fcntl(serverSocketFd, F_GETFL, 0);
	fcntl(serverSocketFd, F_SETFL, flags | O_NONBLOCK);
	addFdToEpoll(serverSocketFd, EPOLLIN);

	epoll_event readyEvents[MAX_EVENTS];
	while (running_)
	{
		int ev_count = epoll_wait(epollFd_, readyEvents, MAX_EVENTS, 1000);
		for (int i = 0; i < ev_count; ++i)
		{
			int sockFd = readyEvents[i].data.fd;
			if (readyEvents[i].events & (EPOLLERR | EPOLLHUP))	// сначала обрабатываем ошибки/закрытие
			{
				closeConnection(sockFd);
				continue;   // пропускаем обработку чтения для этого сокета
			}
			if (readyEvents[i].events & EPOLLIN)	// затем обрабатываем готовность к чтению
			{
				if (sockFd == serverSocketFd)
					handleNewConnection(sockFd);
				else
					handleClientData(sockFd);
			}
		}
	}
}
