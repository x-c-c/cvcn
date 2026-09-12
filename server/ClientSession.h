#pragma once
#include "PacketData.h"
#include "PacketAssembler.h"
#include "ResponseSender.h"
#include <sys/socket.h>
#include "Database.h"

class Epoller;

class ClientSession
{
public:
	ClientSession(int fileDescriptor, Epoller* epoller, Database* db);
	~ClientSession();

	void handleRead();
	void handleWrite();
	void closeSession();

	int getfileDescriptor() const { return fileDescriptor_; }
	bool isClosed() const { return closed_; }

private:
	static constexpr size_t TEMP_BUFFER_SIZE = 4096;
	int fileDescriptor_;
	bool closed_ = false;
	Epoller* epoller_;
	Database* db_;
	PacketAssembler assembler_;
	ResponseSender sender_;

	void processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
	void handleConnectRequestData(uint32_t messageID, uint32_t sessionID);
	void handleRegisterRequestData(uint32_t messageID, uint32_t sessionID, const RegisterRequestData& data);
	void handleAuthRequestData(uint32_t messageID, uint32_t sessionID, const AuthRequestData& data);
	void handleMessageSendData(uint32_t messageID, uint32_t sessionID, const MessageSendData& data);
	void handleDisconnectRequestData();
	void handleDeleteRequestData(uint32_t messageID, uint32_t sessionID, const DeleteRequestData& data);
	
	bool validateIncomingPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
};
