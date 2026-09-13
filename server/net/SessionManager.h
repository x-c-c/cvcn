#pragma once
#include <unordered_map>
#include <cstdint>

class ClientSession;
class PacketDispatcher;
class SessionRegistry;
class EventPoller;

class SessionManager
{
public:
    SessionManager(PacketDispatcher* dispatcher,
                   SessionRegistry* sessionRegistry,
                   EventPoller* eventPoller);
    ~SessionManager();

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    void onNewConnection(int fileDescriptor);
    void onRead(int fileDescriptor);
    void onWrite(int fileDescriptor);
    void onError(int fileDescriptor, uint32_t events);

private:
    PacketDispatcher* dispatcher_;
    SessionRegistry* sessionRegistry_;
    EventPoller* eventPoller_;
    std::unordered_map<int, ClientSession*> sessions_;

    void closeClient(int fileDescriptor);
};
