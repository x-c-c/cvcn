#include "../utils/Logger.h"
#include "../config/ServerConfig.h"
#include "../net/PortSelector.h"
#include "../net/ListeningSocket.h"
#include "./ShutdownSignal.h"
#include "../storage/Database.h"
#include "../net/EventPoller.h"
int main()
{
	ShutdownSignal::setup();
	Logger::instance().info("Server starting up");
	Database db("chat.db");
	ServerConfig config;
	int chosenPort = getValidPort(config.getPort());
	if (chosenPort == -1)
	{
		Logger::instance().info("Shutdown requested during port selection");
		return 0;
	}
	config.setPort(chosenPort);
	ListeningSocket server;
	server.start(config);
	
	EventPoller epoller(&db);
	epoller.startEpollLoop(server.getServerSocketFD());
	
	
	Logger::instance().info("Server shutdown");
	return 0;
}
