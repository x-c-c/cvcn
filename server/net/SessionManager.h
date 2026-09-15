#pragma once
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

class ClientSession;
class PacketDispatcher;
class SessionRegistry;
class EventPoller;
class ThreadPool;
class ResultQueue;

class SessionManager
{
public:
    SessionManager(PacketDispatcher* dispatcher,
                   SessionRegistry* sessionRegistry,
                   EventPoller* eventPoller,
                   ThreadPool* threadPool,
                   ResultQueue* resultQueue);
    ~SessionManager();

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    void onNewConnection(int fileDescriptor);
    void onRead(int fileDescriptor);
    void onWrite(int fileDescriptor);
    void onError(int fileDescriptor, uint32_t events);

    /** @brief Обработать команду Close из ResultQueue. main-thread. */
    void requestCloseClient(int fileDescriptor);

    /** @brief Уменьшить in-flight по fd. main-thread. */
    void onTaskDone(int fileDescriptor);

    ClientSession* getSession(int fileDescriptor);

private:
    PacketDispatcher* dispatcher_;
    SessionRegistry* sessionRegistry_;
    EventPoller* eventPoller_;
    ThreadPool* threadPool_;
    ResultQueue* resultQueue_;
    std::unordered_map<int, ClientSession*> sessions_;
    std::unordered_map<int, int> inFlight_;
    std::unordered_set<int> pendingDelete_;

    void closeClient(int fileDescriptor);
    void closeOrDefer(int fileDescriptor);
};
