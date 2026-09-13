#pragma once
#include "PacketData.h"
#include "PacketAssembler.h"
#include "ResponseSender.h"
#include <sys/socket.h>
#include <string>
#include <vector>

class Epoller;
class PacketDispatcher;

class ClientSession
{
public:
	ClientSession(int fileDescriptor,
				  Epoller* epoller,
				  PacketDispatcher* dispatcher);
	~ClientSession();

	void handleRead();
	void handleWrite();
	void closeSession();

	int getfileDescriptor() const { return fileDescriptor_; }
	bool isClosed() const { return closed_; }

	int getUserID() const { return userID_; }
	const std::string& getUsername() const { return username_; }
	void setAuthenticated(int userID, const std::string& username);

	void sendRaw(const std::vector<uint8_t>& data) { sender_.sendResponse(data); }

private:
	static constexpr size_t TEMP_BUFFER_SIZE = 4096;

	int fileDescriptor_;
	bool closed_ = false;
	Epoller* epoller_;
	PacketDispatcher* dispatcher_;
	PacketAssembler assembler_;
	ResponseSender sender_;

	int userID_ = -1;
	std::string username_;

	void processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
};
