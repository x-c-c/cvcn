#include "Logger.h"
#include "ServerConfig.h"
#include "CheckPort.h"
#include "ServerStartStop.h"
#include "SigintHandler.h"
#include "Database.h"

int main()
{
	SigintHandler::setup();
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
	ServerStartStop server;
	server.start(config, &db);
	Logger::instance().info("Server shutdown");
	return 0;
}
