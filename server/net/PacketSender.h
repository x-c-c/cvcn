#pragma once
#include <vector>
#include <cstdint>
#include <deque>
#include <sys/socket.h>

class EventPoller;

class PacketSender
{
public:
	PacketSender(EventPoller* eventPoller, int fileDescriptor);

	/** @brief Поставить пакет в очередь и попробовать отправить. */
	void sendPacket(const std::vector<uint8_t>& data);

	/** @brief Вызывается EventPoller'ом, когда fd готов к записи. */
	void handleWrite();

private:
	EventPoller* eventPoller_;
	int fileDescriptor_;
	std::deque<std::vector<uint8_t>> sendQueue_;
	bool writePending_ = false;

	void flushSendQueue();
	void armWriteNotification();
};
