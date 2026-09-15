#include "SessionManager.h"
#include "ClientSession.h"
#include "PacketDispatcher.h"
#include "SessionRegistry.h"
#include "EventPoller.h"
#include "ThreadPool.h"
#include "ResultQueue.h"
#include "Logger.h"

SessionManager::SessionManager(PacketDispatcher* dispatcher,
                               SessionRegistry* sessionRegistry,
                               EventPoller* eventPoller,
                               ThreadPool* threadPool,
                               ResultQueue* resultQueue):
    dispatcher_(dispatcher),
    sessionRegistry_(sessionRegistry),
    eventPoller_(eventPoller),
    threadPool_(threadPool),
    resultQueue_(resultQueue){}

SessionManager::~SessionManager()
{
    for (auto& pair : sessions_)
    {
        ClientSession* s = pair.second;
        if (!s->isClosed())
        {
            const int userID = s->getUserID();
            if (sessionRegistry_ && userID != -1)
                sessionRegistry_->unregisterUser(userID, s);
            s->closeSession();
        }
        delete s;
    }
    sessions_.clear();
}

ClientSession* SessionManager::getSession(int fileDescriptor)
{
    auto it = sessions_.find(fileDescriptor);
    return (it == sessions_.end()) ? nullptr : it->second;
}

void SessionManager::onNewConnection(int fileDescriptor)
{
    ClientSession* session = new ClientSession(
        fileDescriptor, eventPoller_, dispatcher_, resultQueue_);
    sessions_[fileDescriptor] = session;
    inFlight_[fileDescriptor] = 0;
    LOG_INFO("New client registered, fd={}", fileDescriptor);
}

void SessionManager::onRead(int fileDescriptor)
{
    auto it = sessions_.find(fileDescriptor);
    if (it == sessions_.end())
        return;

    ClientSession* session = it->second;
    std::vector<Task> tasks = session->handleRead();

    if (!tasks.empty() && threadPool_ && resultQueue_)
    {
        for (Task& task : tasks)
        {
            ++inFlight_[fileDescriptor];
            ClientSession* s = task.session;
            int fd = fileDescriptor;
            PacketHeaderRaw header = task.header;
            std::vector<uint8_t> body = std::move(task.body);

            threadPool_->submit([this, s, fd, header, body]() {
                try
                {
                    if (!s->isClosed())
                        dispatcher_->dispatch(header, body, s);
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("Exception in worker for fd {}: {}", fd, e.what());
                }
                catch (...)
                {
                    LOG_ERROR("Unknown exception in worker for fd {}", fd);
                }

                SessionCommand done;
                done.type = CommandType::TaskDone;
                done.fd = fd;
                resultQueue_->push(std::move(done));
            });
        }
    }

    if (session->isClosed())
        closeOrDefer(fileDescriptor);
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
    closeOrDefer(fileDescriptor);
}

void SessionManager::requestCloseClient(int fileDescriptor)
{
    auto it = sessions_.find(fileDescriptor);
    if (it == sessions_.end())
        return;

    if (inFlight_[fileDescriptor] > 0)
    {
        pendingDelete_.insert(fileDescriptor);
        if (!it->second->isClosed())
            it->second->closeSession();
        return;
    }

    closeClient(fileDescriptor);
}

void SessionManager::onTaskDone(int fileDescriptor)
{
    auto it = inFlight_.find(fileDescriptor);
    if (it == inFlight_.end())
        return;

    if (it->second > 0)
        --it->second;

    if (it->second == 0 && pendingDelete_.count(fileDescriptor))
    {
        pendingDelete_.erase(fileDescriptor);
        closeClient(fileDescriptor);
    }
}

void SessionManager::closeOrDefer(int fileDescriptor)
{
    auto inflightIt = inFlight_.find(fileDescriptor);
    const int pending = (inflightIt == inFlight_.end()) ? 0 : inflightIt->second;

    if (pending > 0)
    {
        pendingDelete_.insert(fileDescriptor);
        auto sit = sessions_.find(fileDescriptor);
        if (sit != sessions_.end() && !sit->second->isClosed())
            sit->second->closeSession();
        return;
    }
    closeClient(fileDescriptor);
}

void SessionManager::closeClient(int fileDescriptor)
{
    auto it = sessions_.find(fileDescriptor);
    if (it == sessions_.end())
        return;

    ClientSession* session = it->second;

    const int userID = session->getUserID();
    if (sessionRegistry_ && userID != -1)
        sessionRegistry_->unregisterUser(userID, session);

    if (!session->isClosed())
        session->closeSession();

    delete session;
    sessions_.erase(it);
    inFlight_.erase(fileDescriptor);
}
