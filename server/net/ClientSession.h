#pragma once
#include "IClientSession.h"
#include "PacketData.h"
#include "PacketAssembler.h"
#include "PacketSender.h"
#include "Task.h"
#include <sys/socket.h>
#include <atomic>
#include <string>
#include <vector>

class EventPoller;
class PacketDispatcher;
class ResultQueue;

/**
 * @file ClientSession.h
 * @brief Одна клиентская сессия.
 *
 * handleRead вызывается из main-thread: читает байты, извлекает пакеты,
 * возвращает vector<Task> для отправки в ThreadPool.
 *
 * sendRaw/requestClose вызываются из воркеров: не отправляют данные
 * напрямую, а кладут команду в ResultQueue (кроме sendRawDirect,
 * которая используется только main-thread при обработке очереди).
 */
class ClientSession : public IClientSession
{
public:
    ClientSession(int fileDescriptor,
                  EventPoller* eventPoller,
                  PacketDispatcher* dispatcher,
                  ResultQueue* resultQueue);
    ~ClientSession() override;

    /** @brief Прочитать доступные байты, вернуть список задач. main-thread. */
    std::vector<Task> handleRead();
    void handleWrite();

    // IClientSession (thread-safe)
    void sendRaw(const std::vector<uint8_t>& data) override;
    void requestClose() override;
    int getFileDescriptor() const override { return fileDescriptor_; }
    int getUserID() const override { return userID_; }
    const std::string& getUsername() const override { return username_; }
    void setAuthenticated(int userID, const std::string& username) override;
    bool isClosed() const override { return closed_.load(); }

    /** @brief Немедленно отправить байты. Только main-thread. */
    void sendRawDirect(const std::vector<uint8_t>& data);

    /** @brief Закрыть сессию. Только main-thread. */
    void closeSession();

private:
    int fileDescriptor_;
    std::atomic<bool> closed_{false};
    EventPoller* eventPoller_;
    PacketDispatcher* dispatcher_;
    ResultQueue* resultQueue_;
    PacketAssembler assembler_;
    PacketSender sender_;

    int userID_ = -1;
    std::string username_;
};
