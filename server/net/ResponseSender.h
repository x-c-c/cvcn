#pragma once
#include <vector>
#include <cstdint>
#include <deque>
#include <sys/socket.h>

class Epoller;

class ResponseSender
{
public:
	ResponseSender(Epoller* epoller, int socketDescriptor);
	void sendResponse(const std::vector<uint8_t>& data);
	void handleWrite();

private:
	Epoller* epoller_;
	int socketDescriptor_;
	std::deque<std::vector<uint8_t>> sendQueue_;
	bool writePending_ = false;

	void flushSendQueue();
	void armWriteNotification();
};
