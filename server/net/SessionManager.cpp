#include "SessionManager.h"
#include "ClientSession.h"
#include "PacketDispatcher.h"
#include "SessionRegistry.h"
#include "EventPoller.h"
#include "Logger.h"

SessionManager::SessionManager(PacketDispatcher* dispatcher,
                               SessionRegistry* sessionRegistry,
                               EventPoller* eventPoller):
    dispatcher_(dispatcher),
    sessionRegistry_(sessionRegistry),
    eventPoller_(eventPoller){}

SessionManager::~SessionManager()
{
    for (auto& pair : sessions_)
    {
        if (!pair.second->isClosed())
        {
            const int userID = pair.second->getUserID();
            if (sessionRegistry_ && userID != -1)
                sessionRegistry_->unregisterUser(userID);
            pair.second->closeSession();
        }
        delete pair.second;
    }
    sessions_.clear();
}

void SessionManager::onNewConnection(int fileDescriptor)
{
    sessions_[fileDescriptor] = new ClientSession(fileDescriptor, eventPoller_, dispatcher_);
    LOG_INFO("New client registered, fd={}", fileDescriptor);
}

void SessionManager::onRead(int fileDescriptor)
{
    auto it = sessions_.find(fileDescriptor);
    if (it == sessions_.end())
        return;
    it->second->handleRead();
    if (it->second->isClosed())
        closeClient(fileDescriptor);
}

void SessionManager::onWrite(int fileDescriptor)
{
    auto it = sessions_.find(fileDescriptor);
    if (it == sessions_.end())
        return;
    it->second->handleWrite();
    if (it->second->isClosed())
        closeClient(fileDescriptor);
}

void SessionManager::onError(int fileDescriptor, uint32_t events)
{
    LOG_WARN("Error event on fd {} (events=0x{:X})", fileDescriptor, events);
    closeClient(fileDescriptor);
}

void SessionManager::closeClient(int fileDescriptor)
{
    auto it = sessions_.find(fileDescriptor);
    if (it == sessions_.end())
        return;

    const int userID = it->second->getUserID();
    if (sessionRegistry_ && userID != -1)
        sessionRegistry_->unregisterUser(userID);

    if (!it->second->isClosed())
        it->second->closeSession();

    delete it->second;
    sessions_.erase(it);
}
