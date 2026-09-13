#pragma once
#include "PacketData.h"
#include "PacketAssembler.h"
#include "PacketSender.h"
#include <sys/socket.h>
#include <string>
#include <vector>

class EventPoller;
class PacketDispatcher;

class ClientSession
{
public:
	ClientSession(int fileDescriptor,
				  EventPoller* epoller,
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

	void sendRaw(const std::vector<uint8_t>& data) { sender_.send(data); }

private:
	static constexpr size_t TEMP_BUFFER_SIZE = 4096;

	int fileDescriptor_;
	bool closed_ = false;
	EventPoller* epoller_;
	PacketDispatcher* dispatcher_;
	PacketAssembler assembler_;
	PacketSender sender_;

	int userID_ = -1;
	std::string username_;

	void processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
};
