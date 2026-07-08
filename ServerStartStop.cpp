#include "ServerStartStop.h"
#include <iostream>
#include "Epoller.h"
void ServerStartStop::start(const ServerConfig& config)
{
	// socket, bind, listen
	serverSocketFileDescriptor	= socket(config.getDomain(), config.getType(), config.getProtocol());
	
	int opt = 1;
	setsockopt(serverSocketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	sockaddr_in	serverAddr{};
	serverAddr.sin_family		= config.getDomain();
	serverAddr.sin_addr.s_addr	= config.getAddr();
	serverAddr.sin_port			= htons(config.getPort()); 
	
	sockaddr*	serverAddrPtr	= reinterpret_cast<sockaddr*>(&serverAddr);
	socklen_t	serverAddrLen	= sizeof(serverAddr);
	bind(serverSocketFileDescriptor, serverAddrPtr, serverAddrLen);
	listen(serverSocketFileDescriptor, SOMAXCONN);
	
	Epoller epoller{};
	epoller.startEpollLoop(serverSocketFileDescriptor);

	close(serverSocketFileDescriptor);
}

void ServerStartStop::stop(){}
