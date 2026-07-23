/**
 * @file ServerStartStop.cpp
 * @brief Реализация запуска сервера.
 */

#include "ServerStartStop.h"
#include "Epoller.h"

void ServerStartStop::initServerAddr(const ServerConfig& config)
{
	serverAddr.sin_family      = config.getDomain();
	serverAddr.sin_addr.s_addr = config.getAddr();
	serverAddr.sin_port        = htons(config.getPort());
}

void ServerStartStop::start(const ServerConfig& config)
{
	serverSocketFileDescriptor = socket(config.getDomain(), config.getType(), config.getProtocol());
	setsockopt(serverSocketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &reuseAddrOption, sizeof(reuseAddrOption));
	initServerAddr(config);
	bind(serverSocketFileDescriptor, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
	listen(serverSocketFileDescriptor, SOMAXCONN);
	
	Epoller epoller;
	epoller.startEpollLoop(serverSocketFileDescriptor);

	close(serverSocketFileDescriptor);
}

void ServerStartStop::stop() {}



