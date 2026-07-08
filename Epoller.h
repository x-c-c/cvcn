#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

class Epoller
{
private:
	static constexpr int MAX_EVENTS = 1024;
	int epollFd_;
	bool running_;

	void addFdToEpoll(int fd, uint32_t events);
	void removeFdFromEpoll(int fd);
	void handleNewConnection(int serverFd);
	void handleClientData(int clientFd);
	void closeConnection(int fd);
	
public:
	Epoller();
	~Epoller();

	void startEpollLoop(int serverSocketFileDescriptor);
	void stoptEpollLoop();
};
