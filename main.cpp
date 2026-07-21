/**
 * @file main.cpp
 * @brief Точка входа сервера.
 */

#include "ServerStartStop.h"

int main(int argc, char* argv[])
{
	ServerConfig config;
	ServerStartStop server;
	server.start(config);
	return 0;
}
