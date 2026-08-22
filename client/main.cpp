#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include "ClientConfig.h"
#include "Logger.h"

sockaddr_in initServerAddr(const ClientConfig& config)
{
	sockaddr_in addr{};
	addr.sin_family			= config.getDomain();
	addr.sin_addr.s_addr	= htonl(config.getAddr());
	addr.sin_port			= htons(config.getPort());
	return addr;
}

int main()
{
	ClientConfig config;
	int clientSocketFd = socket(config.getDomain(), config.getType(), config.getProtocol());
	if (clientSocketFd < 0)
	{
		Logger::instance().critical("socket() failed: {}", strerror(errno));
		return 1;
	}
	sockaddr_in serverAddr = initServerAddr(config);
	if (connect(clientSocketFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		Logger::instance().error("Client connect error: {}", strerror(errno));
		return 1;
	}
	
	close(clientSocketFd);
	Logger::instance().info("Client stopped");
	return 0;
}
