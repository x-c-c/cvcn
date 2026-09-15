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
    /**
     * @brief Создать сессию для принятого сокета.
     *
     * @param fileDescriptor fd клиента (non-blocking)
     * @param eventPoller    epoll-обёртка; не владеет
     * @param dispatcher     обработчик пакетов; не владеет
     * @param resultQueue    очередь команд в main-thread; не владеет
     */

    ClientSession(int fileDescriptor,
                  EventPoller* eventPoller,
                  PacketDispatcher* dispatcher,
                  ResultQueue* resultQueue);
    /**
     * @brief Закрыть сессию, если она ещё открыта.
     * @note Вызывается только из main-thread (SessionManager).
     */

    ~ClientSession() override;

    /** @brief Прочитать доступные байты, вернуть список задач. main-thread. */
    std::vector<Task> handleRead();
    /**
     * @brief Возобновить отправку после EPOLLOUT.
     * @note Только main-thread.
     */

    void handleWrite();

    // IClientSession (thread-safe)
    /**
     * @brief Асинхронно отправить пакет клиенту.
     * @param data пакет целиком (заголовок + тело)
     * @post В ResultQueue лежит команда SendRaw.
     * @note Безопасно из любого потока.
     */

    void sendRaw(const std::vector<uint8_t>& data) override;
    /**
     * @brief Асинхронно запросить закрытие сессии.
     * @post В ResultQueue лежит команда Close.
     * @note Безопасно из любого потока, идемпотентно.
     */

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
