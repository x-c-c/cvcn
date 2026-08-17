#pragma once
#include "Packets.h"
#include "PacketAssembler.h"
#include "ResponseSender.h"
#include <sys/socket.h>
#include "Database.h"

class Epoller;

class ClientSession
{
public:
    ClientSession(int socketDescriptor, Epoller* epoller, Database* db);
    ~ClientSession();

    void handleRead();
    void handleWrite();
    void closeSession();

    int getSocketDescriptor() const { return socketDescriptor_; }
    bool isClosed() const { return closed_; }

private:
    int socketDescriptor_;
    bool closed_ = false;
    Epoller* epoller_;
    Database* db_;
    PacketAssembler assembler_;
    ResponseSender sender_;

    void processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
    void handleConnectRequestPacket(uint32_t messageID, uint32_t sessionID);
    void handleRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet);
    void handleAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet);
    void handleMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet);
    void handleDisconnectRequestPacket();
};


