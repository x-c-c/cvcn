#include "ServerStartStop.h"
#include "Epoller.h"
#include "Logger.h"
#include "Database.h"
#include <cstring>

void ServerStartStop::initServerAddr(const ServerConfig& config)
{
	serverAddr.sin_family      = config.getDomain();
	serverAddr.sin_addr.s_addr = config.getAddr();
	serverAddr.sin_port        = htons(config.getPort());
}

void ServerStartStop::start(const ServerConfig& config, Database* db)
{
	serverSocketFileDescriptor = socket(config.getDomain(), config.getType(), config.getProtocol());
	if (serverSocketFileDescriptor < 0)
	{
		Logger::instance().critical("socket() failed: {}", strerror(errno));
		return;
	}
	setsockopt(serverSocketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &reuseAddrOption, sizeof(reuseAddrOption));
	initServerAddr(config);
	if (bind(serverSocketFileDescriptor, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) != 0)
	{
		Logger::instance().critical("bind() on port {} failed: {}", config.getPort(), strerror(errno));
		close(serverSocketFileDescriptor);
		return;
	}
	listen(serverSocketFileDescriptor, SOMAXCONN);
	Logger::instance().info("Server listening on port {}", config.getPort());

	Epoller epoller(db);
	epoller.startEpollLoop(serverSocketFileDescriptor);

	close(serverSocketFileDescriptor);
	Logger::instance().info("Server stopped");
}

void ServerStartStop::stop(){}	//потом добавлю
