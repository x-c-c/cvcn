/**
 * @file ServerStartStop.cpp
 * @brief Реализация запуска сервера.
 */

#include "ServerStartStop.h"
#include "Logger.h"
#include <csignal>

Epoller* ServerStartStop::currentEpoller_ = nullptr;

void ServerStartStop::handleSignal(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
	{
		Logger::instance().warn("Received signal {}, shutting down", signum);
		if (currentEpoller_)
			currentEpoller_->stopEpollLoop();
	}
}

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
	
	Logger::instance().info("Server listening on port {}", config.getPort());

	Epoller epoller;
	currentEpoller_ = &epoller;							// сохраняем указатель для остановки по сигналу
	epoller.startEpollLoop(serverSocketFileDescriptor);
	currentEpoller_ = nullptr;

	close(serverSocketFileDescriptor);
	Logger::instance().info("Server stopped");
}



