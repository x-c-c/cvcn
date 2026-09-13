#pragma once
#include "IClientSession.h"
#include "PacketData.h"
#include "PacketAssembler.h"
#include "PacketSender.h"
#include <sys/socket.h>
#include <string>
#include <vector>

class EventPoller;
class PacketDispatcher;

/**
 * @file ClientSession.h
 * @brief Одна клиентская сессия: сокет, буфер, состояние аутентификации.
 *
 * Владеет:
 *   - int fileDescriptor_
 *   - PacketAssembler assembler_
 *   - PacketSender sender_
 * Не владеет: EventPoller*, PacketDispatcher*.
 */
class ClientSession : public IClientSession
{
public:
    ClientSession(int fileDescriptor,
                  EventPoller* eventPoller,
                  PacketDispatcher* dispatcher);
    ~ClientSession() override;

    void handleRead();
    void handleWrite();

    // IClientSession
    void sendRaw(const std::vector<uint8_t>& data) override;
    int getFileDescriptor() const override { return fileDescriptor_; }
    int getUserID() const override { return userID_; }
    const std::string& getUsername() const override { return username_; }
    void setAuthenticated(int userID, const std::string& username) override;
    void closeSession() override;
    bool isClosed() const override { return closed_; }

private:
    int fileDescriptor_;
    bool closed_ = false;
    EventPoller* eventPoller_;
    PacketDispatcher* dispatcher_;
    PacketAssembler assembler_;
    PacketSender sender_;

    int userID_ = -1;
    std::string username_;

    void processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
};
