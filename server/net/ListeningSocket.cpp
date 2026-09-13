#include "ListeningSocket.h"
#include "Logger.h"
#include <cstring>
ListeningSocket::~ListeningSocket()
{
	closeSocket();
}
void ListeningSocket::initServerAddr(const ServerConfig& config)
{
	serverAddr.sin_family      = config.getDomain();
	serverAddr.sin_addr.s_addr = config.getAddr();
	serverAddr.sin_port        = htons(config.getPort());
}

void ListeningSocket::listen(const ServerConfig& config)
{
	if (serverSocketFD_ != -1)
	{
		Logger::instance().warn("Server socket already open, closing it first");
		closeSocket();
	}

	serverSocketFD_ = socket(config.getDomain(), config.getType(), config.getProtocol());
	if (serverSocketFD_ < 0)
	{
		Logger::instance().critical("socket() failed: {}", strerror(errno));
		closeSocket();
		return;
	}
	setsockopt(serverSocketFD_, SOL_SOCKET, SO_REUSEADDR, &reuseAddrOption, sizeof(reuseAddrOption));
	initServerAddr(config);
	if (bind(serverSocketFD_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) != 0)
	{
		Logger::instance().critical("bind() on port {} failed: {}", config.getPort(), strerror(errno));
		closeSocket();
		return;
	}

	if (listen(serverSocketFD_, SOMAXCONN) != 0)
	{
		Logger::instance().critical("listen() failed: {}", strerror(errno));
		closeSocket();
		return;
	}
	
	Logger::instance().info("Server listening on port {}", config.getPort());


}

void ListeningSocket::closeSocket()
{
	if (serverSocketFD_ != -1)
	{
		close(serverSocketFD_);
		serverSocketFD_ = -1;
	}
	Logger::instance().info("Closing server socket");
}
