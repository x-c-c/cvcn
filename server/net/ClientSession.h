#pragma once
#include "Packets.h"
#include "../protocol/PacketAssembler.h"
#include "../protocol/PacketSender.h"
#include <sys/socket.h>
#include "../storage/Database.h"

class EventPoller;

class ClientSession
{
public:
    ClientSession(int socketDescriptor, EventPoller* epoller, Database* db);
    ~ClientSession();

    void handleRead();
    void handleWrite();
    void closeSession();

    int getSocketDescriptor() const { return socketDescriptor_; }
    bool isClosed() const { return closed_; }

private:
	int TEMP_BUFFER_SIZE = 4096;
    int socketDescriptor_;
    bool closed_ = false;
    EventPoller* epoller_;
    Database* db_;
    PacketAssembler assembler_;
    PacketSender sender_;

    void processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
    void handleConnectRequestPacket(uint32_t messageID, uint32_t sessionID);
    void handleRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet);
    void handleAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet);
    void handleMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet);
    void handleDisconnectRequestPacket();
};


