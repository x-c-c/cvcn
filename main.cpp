/**
 * @file main.cpp
 * @brief Точка входа сервера.
 */ 
#include "Logger.h"
#include "ServerConfig.h"
#include "CheckPort.h"
#include "ServerStartStop.h"
#include "SigintHandler.h"

int main()
{
	SigintHandler::setup();

	Logger::instance().info("Server starting up");
	ServerConfig config;
	config.setPort(getValidPort(config.getPort()));
	ServerStartStop server;
	server.start(config);
	Logger::instance().info("Server shutdown");
	return 0;
}
