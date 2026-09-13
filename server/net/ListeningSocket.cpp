#include "ListeningSocket.h"
#include "Logger.h"
#include <cstring>

ListeningSocket::~ListeningSocket()
{
	closeSocket();
}

void ListeningSocket::setupServerAddress(const ServerConfig& config)
{
	serverAddr_.sin_family      = config.getDomain();
	serverAddr_.sin_addr.s_addr = config.getAddr();
	serverAddr_.sin_port        = htons(config.getPort());
}

void ListeningSocket::startListening(const ServerConfig& config)
{
	if (serverFileDescriptor_ != -1)
	{
		Logger::instance().warn("Listening socket already open, closing it first");
		closeSocket();
	}

	serverFileDescriptor_ = socket(config.getDomain(), config.getType(), config.getProtocol());
	if (serverFileDescriptor_ < 0)
	{
		Logger::instance().critical("socket() failed: {}", strerror(errno));
		closeSocket();
		return;
	}

	setsockopt(serverFileDescriptor_, SOL_SOCKET, SO_REUSEADDR, &reuseAddrOption, sizeof(reuseAddrOption));
	setupServerAddress(config);

	if (bind(serverFileDescriptor_, reinterpret_cast<sockaddr*>(&serverAddr_), sizeof(serverAddr_)) != 0)
	{
		Logger::instance().critical("bind() on port {} failed: {}", config.getPort(), strerror(errno));
		closeSocket();
		return;
	}

	if (listen(serverFileDescriptor_, SOMAXCONN) != 0)
	{
		Logger::instance().critical("listen() failed: {}", strerror(errno));
		closeSocket();
		return;
	}

	Logger::instance().info("Server listening on port {}", config.getPort());
}

void ListeningSocket::closeSocket()
{
	if (serverFileDescriptor_ != -1)
	{
		close(serverFileDescriptor_);
		serverFileDescriptor_ = -1;
		Logger::instance().info("Closing server socket");
	}
}
