#include "ListeningSocket.h"
#include "Logger.h"
#include <cstring>

ListeningSocket::~ListeningSocket()
{
	closeSocket();
}

void ListeningSocket::setupServerAddress(const ServerConfig& config)
{
	serverAddress_.sin_family      = config.getDomain();
	serverAddress_.sin_addr.s_addr = config.getAddr();
	serverAddress_.sin_port        = htons(config.getPort());
}

void ListeningSocket::startListening(const ServerConfig& config)
{
	if (serverFileDescriptor_ != -1)
	{
		LOG_WARN("Listening socket already open, closing it first");
		closeSocket();
	}

	serverFileDescriptor_ = socket(config.getDomain(), config.getType(), config.getProtocol());
	if (serverFileDescriptor_ < 0)
	{
		LOG_CRITICAL("socket() failed: {}", strerror(errno));
		closeSocket();
		return;
	}

	setsockopt(serverFileDescriptor_, SOL_SOCKET, SO_REUSEADDR, &reuseAddrOption, sizeof(reuseAddrOption));
	setupServerAddress(config);

	if (bind(serverFileDescriptor_, reinterpret_cast<sockaddr*>(&serverAddress_), sizeof(serverAddress_)) != 0)
	{
		LOG_CRITICAL("bind() on port {} failed: {}", config.getPort(), strerror(errno));
		closeSocket();
		return;
	}

	if (listen(serverFileDescriptor_, SOMAXCONN) != 0)
	{
		LOG_CRITICAL("listen() failed: {}", strerror(errno));
		closeSocket();
		return;
	}

	LOG_INFO("Server listening on port {}", config.getPort());
}

void ListeningSocket::closeSocket()
{
	if (serverFileDescriptor_ != -1)
	{
		close(serverFileDescriptor_);
		serverFileDescriptor_ = -1;
		LOG_INFO("Closing server socket");
	}
}
