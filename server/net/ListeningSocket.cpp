#include "./ListeningSocket.h"
#include "../utils/Logger.h"
#include <cstring>
#include <stdexcept>
#include <string>
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

void ListeningSocket::start(const ServerConfig& config)
{
	if (serverSocketFD_ != -1)
	{
		Logger::instance().warn("Server socket already open, closing it first");
		closeSocket();
	}

	serverSocketFD_ = socket(config.getDomain(), config.getType(), config.getProtocol());
	if (serverSocketFD_ < 0)
	{
		closeSocket();
		throw std::runtime_error( std::string("socket() failed: ") + strerror(errno));
	}
	setsockopt(serverSocketFD_, SOL_SOCKET, SO_REUSEADDR, &reuseAddrOption, sizeof(reuseAddrOption));
	initServerAddr(config);
	if (bind(serverSocketFD_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) != 0)
	{
		closeSocket();
		throw std::runtime_error( std::string("socket() failed: ") + strerror(errno));
	}

	if (listen(serverSocketFD_, SOMAXCONN) != 0)
	{
		closeSocket();
		throw std::runtime_error( std::string("socket() failed: ") + strerror(errno));
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
