#pragma once
#include <sys/epoll.h>
#include <atomic>
#include <functional>

class Epoller
{
public:
	Epoller();
	~Epoller();

	
	using newConnectionCallback = std::function<void(int clientSocketFD)>;
	using readEventCallback = std::function<void(int fileDescriptor)>;
	using writeEventCallback = std::function<void(int fileDescriptor)>;
	using errorEventCallback = std::function<void(int fileDescriptor, uint32_t events)>;
	
	void setNewConnectionCallback(newConnectionCallback cb);	// cb == callback
	void setReadEventCallback(readEventCallback cb);
	void setWriteEventCallback(writeEventCallback cb);
	void setErrorEventCallback(errorEventCallback cb);


	void startEpollLoop(int serverSocketFD);
	void stopEpollLoop();
	void modifyFdEvents(int fileDescriptor, uint32_t events);
	//void closeClient(int fileDescriptor);
	void addFdToEpoll(int fileDescriptor, uint32_t events);
	void removeFdFromEpoll(int fileDescriptor);

private:
	// возможно стоит поменять имя
	// сейчас это объект типа std::function<void(...)>; который хранит коллбэк на какую-либо функцию
	newConnectionCallback onNewConnection_;
	readEventCallback onRead_;
	writeEventCallback onWrite_;
	errorEventCallback onError_;
	
	int epollFD_ = -1;
	std::atomic<bool> running_{false};
		
	static constexpr int MAX_EVENTS = 1024;
	static constexpr int WAIT_MILLISECONDS = 1000;
};
