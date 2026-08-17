#pragma once
#include <sys/epoll.h>
#include <unordered_map>
#include <atomic>

class ClientSession;
class Database;

class Epoller
{
public:
	explicit Epoller(Database* db);
	~Epoller();

	void startEpollLoop(int serverSocketDescriptor);
	void stopEpollLoop();
	void modifyFdEvents(int fileDescriptor, uint32_t events);
	void closeClient(int fileDescriptor);
	Database* getDatabase() { return db_; }

private:
	static constexpr int MAX_EVENTS = 1024;
	static constexpr int WAIT_MILLISECONDS = 1000;
	int epollFileDescriptor_;
	std::atomic<bool> running_{true};
	std::unordered_map<int, ClientSession*> sessions_;
	Database* db_;

	void addFdToEpoll(int fileDescriptor, uint32_t events);
	void removeFdFromEpoll(int fileDescriptor);
	void handleNewConnection(int serverSocketDescriptor);
};
