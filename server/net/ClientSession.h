#pragma once
#include "../protocol/PacketData.h"
#include "../protocol/PacketAssembler.h"
#include "../protocol/PacketSender.h"
#include <cstddef>
#include <cstdint>
#include <vector>

class EventPoller;
class Database;
class ClientSession
{
public:
    ClientSession(int socketFD, EventPoller* eventPoller, Database* db);
    ~ClientSession();
    ClientSession(const ClientSession&) = delete;
    ClientSession& operator=(const ClientSession&) = delete;
 
    void handleRead();
    void handleWrite();
    void closeSession();

    int getSocketDescriptor() const { return socketFD_; }
    bool isClosed() const { return closed_; }
    
private:
	int TEMP_BUFFER_SIZE = 4096;
    int socketFD_;
    bool closed_ = false;
    EventPoller* eventPoller_;
    Database* db_;
    PacketAssembler assembler_;
    PacketSender sender_;

    void processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
    void handlePacket(uint32_t messageID, uint32_t sessionID, const ConnectRequestData& data);
    void handlePacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestData& data);
    void handlePacket(uint32_t messageID, uint32_t sessionID, const AuthRequestData& data);
    void handlePacket(uint32_t messageID, uint32_t sessionID, const MessageSendData& data);
    void handlePacket();  // DisconnectRequest
};


