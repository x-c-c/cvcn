#pragma once
#include <sys/epoll.h>
#include <unordered_map>
#include <atomic>
class ClientSession;
class Database;
class Epoller
{
public:
	Epoller(Database* db);
	~Epoller();
	
	void startEpollLoop(int serverSocketDescriptor);
	void stopEpollLoop();
	void modifyFdEvents(int fileDescriptor, uint32_t events);
	void closeSession(int fileDescriptor);
	Database* getDatabase(){ return db_; }

private:
	
	static constexpr int MAX_EVENTS = 1024;				///< Максимальное число событий за один epoll_wait.
	static constexpr int WAIT_IN_MILLISECOND = 1000;	///< Промежутки времени через которые epoll опрашивает сокеты, измеряется в миллисекундах.
	int epollFileDescriptor_;							///< Файловый дескриптор epoll.
	std::atomic<bool> running_{true};					///< Флаг работы главного цикла.
	std::unordered_map<int, ClientSession*> sessions_;	///< Карта активных сессий (fd → объект).
	Database* db_;
	
	void addFdToEpoll(int fileDescriptor, uint32_t events);
	void removeFdFromEpoll(int fileDescriptor);
	void handleNewConnection(int serverSocketDescriptor);
};
