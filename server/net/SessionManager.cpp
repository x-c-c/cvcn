#include "SessionManager.h"
#include "ClientSession.h"
#include "PacketDispatcher.h"
#include "SessionRegistry.h"
#include "Epoller.h"
#include "Logger.h"

SessionManager::SessionManager(PacketDispatcher* dispatcher,
							   SessionRegistry* sessionRegistry,
							   Epoller* epoller):
	dispatcher_(dispatcher),
	sessionRegistry_(sessionRegistry),
	epoller_(epoller){}

SessionManager::~SessionManager()
{
	for (auto& pair : sessions_)
	{
		if (!pair.second->isClosed())
		{
			const int uid = pair.second->getUserID();
			if (sessionRegistry_ && uid != -1)
				sessionRegistry_->unregisterUser(uid);
			pair.second->closeSession();
		}
	}
	sessions_.clear();
}

void SessionManager::onNewConnection(int fileDescriptor)
{
	sessions_[fileDescriptor] = std::make_unique<ClientSession>(
		fileDescriptor, epoller_, dispatcher_);
	Logger::instance().info("New client registered, fd={}", fileDescriptor);
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
	Logger::instance().warn("Error event on fd {} (events=0x{:X})", fileDescriptor, events);
	closeClient(fileDescriptor);
}

void SessionManager::closeClient(int fileDescriptor)
{
	auto it = sessions_.find(fileDescriptor);
	if (it == sessions_.end())
		return;

	const int uid = it->second->getUserID();
	if (sessionRegistry_ && uid != -1)
		sessionRegistry_->unregisterUser(uid);

	if (!it->second->isClosed())
		it->second->closeSession();

	sessions_.erase(it);
}
