#pragma once
#include <sys/epoll.h>
#include <atomic>
#include <functional>

class EventPoller
{
public:
	EventPoller();
	~EventPoller();

	
	using NewConnectionCallback = std::function<void(int fileDescriptor)>;
	using ReadEventCallback = std::function<void(int fileDescriptor)>;
	using WriteEventCallback = std::function<void(int fileDescriptor)>;
	using ErrorEventCallback = std::function<void(int fileDescriptor, uint32_t events)>;
	
	void setNewConnectionCallback(NewConnectionCallback callback);
	void setReadEventCallback(ReadEventCallback callback);
	void setWriteEventCallback(WriteEventCallback callback);
	void setErrorEventCallback(ErrorEventCallback callback);


	void startEventLoop(int serverFileDescriptor);
	void stopEventLoop();
	void modifyFileDescriptorEvents(int fileDescriptor, uint32_t events);
	void addFileDescriptor(int fileDescriptor, uint32_t events);
	void removeFileDescriptor(int fileDescriptor);

private:
	// возможно стоит поменять имя
	// сейчас это объект типа std::function<void(...)>; который хранит коллбэк на какую-либо функцию
	NewConnectionCallback onNewConnection_;
	ReadEventCallback onRead_;
	WriteEventCallback onWrite_;
	ErrorEventCallback onError_;
	
	int epollFD_ = -1;
	std::atomic<bool> running_{false};
		
	static constexpr int MAX_EVENTS = 1024;
	static constexpr int WAIT_MILLISECONDS = 1000;
};
