#pragma once
#include <vector>
#include <cstdint>
#include <deque>
#include <sys/socket.h>

class EventPoller;

class PacketSender
{
public:
	PacketSender(EventPoller* epoller, int socketDescriptor);
	void send(const std::vector<uint8_t>& data);
	void handleWrite();

private:
	EventPoller* epoller_;
	int socketDescriptor_;
	std::deque<std::vector<uint8_t>> sendQueue_;
	bool writePending_ = false;

	void flushSendQueue();
	void armWriteNotification();
};
