#include "ResponseSender.h"
#include "Epoller.h"
#include "Logger.h"
#include <cerrno>
#include <unistd.h>

ResponseSender::ResponseSender(Epoller* epoller, int socketDescriptor):
	epoller_(epoller), socketDescriptor_(socketDescriptor){}

void ResponseSender::armWriteNotification()
{
	epoller_->modifyFdEvents(socketDescriptor_, EPOLLIN | EPOLLOUT);
	writePending_ = true;
}

void ResponseSender::flushSendQueue()
{
	while (!sendQueue_.empty())
	{
		std::vector<uint8_t>& buffer = sendQueue_.front();
		ssize_t sent = send(socketDescriptor_, buffer.data(), buffer.size(), MSG_NOSIGNAL);
		if (sent < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				armWriteNotification();
				return;
			}
			Logger::instance().error("send error on fd {}: {}", socketDescriptor_, strerror(errno));
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
	epoller_->modifyFdEvents(socketDescriptor_, EPOLLIN);
	writePending_ = false;
}

void ResponseSender::sendResponse(const std::vector<uint8_t>& data)
{
	sendQueue_.push_back(data);
	if (!writePending_)
		flushSendQueue();
}

void ResponseSender::handleWrite()
{
	writePending_ = false;
	flushSendQueue();
}
