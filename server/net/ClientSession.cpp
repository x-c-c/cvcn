#include "ClientSession.h"
#include "EventPoller.h"
#include "PacketDispatcher.h"
#include "Logger.h"
#include <cstring>
#include <cerrno>
#include <unistd.h>

ClientSession::ClientSession(int fileDescriptor,
							 EventPoller* epoller,
							 PacketDispatcher* dispatcher):
	fileDescriptor_(fileDescriptor),
	epoller_(epoller),
	dispatcher_(dispatcher),
	sender_(epoller, fileDescriptor){}

ClientSession::~ClientSession()
{
	if (!closed_)
		closeSession();
}

void ClientSession::handleRead()
{
	uint8_t tempBuffer[TEMP_BUFFER_SIZE];
	ssize_t bytesRead = recv(fileDescriptor_, tempBuffer, sizeof(tempBuffer), 0);
	if (bytesRead > 0)
	{
		assembler_.appendData(tempBuffer, bytesRead);
		PacketHeaderRaw header;
		std::vector<uint8_t> body;
		while (assembler_.extractPacket(header, body))
		{
			processPacket(header, body);
			if (closed_)
				return;
		}
	}
	else if (bytesRead == 0)
	{
		Logger::instance().info("Client {} closed connection", fileDescriptor_);
		closeSession();
	}
	else if (errno != EAGAIN && errno != EWOULDBLOCK)
	{
		Logger::instance().error("recv error on fd {}: {}", fileDescriptor_, strerror(errno));
		closeSession();
	}
}

void ClientSession::handleWrite()
{
	sender_.handleWrite();
}

void ClientSession::processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
	if (dispatcher_)
		dispatcher_->dispatch(header, body, *this);
}

void ClientSession::setAuthenticated(int userID, const std::string& username)
{
	userID_ = userID;
	username_ = username;
}

void ClientSession::closeSession()
{
	if (closed_)
		return;
	epoller_->removeFdFromEpoll(fileDescriptor_);
	close(fileDescriptor_);
	closed_ = true;
	Logger::instance().info("Session closed for fd {}", fileDescriptor_);
}
