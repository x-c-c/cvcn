/**
 * @file    SessionManager.cpp
 * @brief   Реализация SessionManager.
 *
 *
 * @see SessionManager.h
 */

#include "./SessionManager.h"
#include "./ClientSession.h"
#include "./EventPoller.h"
#include "../utils/Logger.h"

SessionManager::SessionManager(EventPoller& epoller, Database& db):
    epoller_(epoller),
    db_(db)
{
}

SessionManager::~SessionManager()
{
    closeAll();
}

void SessionManager::onNewConnection(int fd)
{
    // Страховка от переиспользования fd: если по нему уже висит сессия,
    // удаляем, иначе перезапишем указатель в карте.
    destroySession(fd);

    ClientSession* session = new ClientSession(fd, &epoller_, &db_);
    sessions_[fd] = session;

    Logger::instance().info("Session created: fd={}", fd);
}

void SessionManager::onRead(int fd)
{
    auto it = sessions_.find(fd);
    if (it == sessions_.end())
        return;     // событие могло прийти после закрытия

    it->second->handleRead();

    // handleRead мог закрыть сессию (EOF, ошибка, DisconnectRequest).
    // Тогда удаляем её из карты прямо сейчас, чтобы не держать мёртвый
    // объект до следующего события.
    eraseIfClosed(fd);
}

void SessionManager::onWrite(int fd)
{
    auto it = sessions_.find(fd);
    if (it == sessions_.end())
        return;

    it->second->handleWrite();
    eraseIfClosed(fd);
}

void SessionManager::onError(int fd, uint32_t events)
{
    Logger::instance().warn("Error event: fd={}, events=0x{:X}", fd, events);
    destroySession(fd);	// destroySession сам закроет fd и уберёт запись из карты.
}

void SessionManager::closeAll()
{
    // Сначала delete всех, потом clear()
    for (auto& pair : sessions_)
    {
        // Деструктор ClientSession сам вызовет closeSession, если сессия ещё не закрыта.
        delete pair.second;
    }
    sessions_.clear();
}

void SessionManager::eraseIfClosed(int fd)
{
    auto it = sessions_.find(fd);
    if (it == sessions_.end())
        return;

    if (!it->second->isClosed())
        return;         // сессия жива, оставляем

    Logger::instance().info("Session removed: fd={}", fd);

    // Порядок важен: delete до erase, иначе итератор станет невалидным.
    delete it->second;
    sessions_.erase(it);
}

void SessionManager::destroySession(int fd)
{
    auto it = sessions_.find(fd);
    if (it == sessions_.end())
        return;

    if (!it->second->isClosed())
        it->second->closeSession();     // закрыть fd, если ещё открыт

    delete it->second;
    sessions_.erase(it);
}
