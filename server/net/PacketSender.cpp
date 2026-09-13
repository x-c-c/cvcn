#include "PacketSender.h"
#include "EventPoller.h"
#include "Logger.h"
#include <cerrno>
#include <unistd.h>

PacketSender::PacketSender(EventPoller* epoller, int socketDescriptor):
	eventPoller_(epoller),
	fileDescriptor_(socketDescriptor){}

void PacketSender::armWriteNotification()
{
	eventPoller_->modifyFileDescriptorEvents(fileDescriptor_, EPOLLIN | EPOLLOUT);
	writePending_ = true;
}

void PacketSender::flushSendQueue()
{
	while (!sendQueue_.empty())
	{
		std::vector<uint8_t>& buffer = sendQueue_.front();

		// Явно ::send — чтобы не путаться с методом класса.
		const ssize_t sent = ::send(fileDescriptor_, buffer.data(), buffer.size(), MSG_NOSIGNAL);

		if (sent < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				armWriteNotification();
				return;
			}
			Logger::instance().error("send error on fd {}: {}", fileDescriptor_, strerror(errno));
			return;
		}

		if (static_cast<size_t>(sent) < buffer.size())
		{
			buffer.erase(buffer.begin(), buffer.begin() + sent);
			armWriteNotification();
			return;
		}

		sendQueue_.pop_front();
	}

	eventPoller_->modifyFileDescriptorEvents(fileDescriptor_, EPOLLIN);
	writePending_ = false;
}

void PacketSender::sendPacket(const std::vector<uint8_t>& data)
{
	sendQueue_.push_back(data);
	if (!writePending_)
		flushSendQueue();
}

void PacketSender::handleWrite()
{
	writePending_ = false;
	flushSendQueue();
}
