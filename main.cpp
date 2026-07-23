/**
 * @file main.cpp
 * @brief Точка входа сервера.
 */

#include "ServerStartStop.h"
#include "ServerConfig.h"
#include "CheckPort.h"

int main()
{
	ServerConfig config;
	config.setPort(getValidPort(config.getPort()));
	ServerStartStop server;
	server.start(config);

	return 0;
}
