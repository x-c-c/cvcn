#pragma once
#include "ServerConfig.h"
#include <sys/socket.h>
#include <unistd.h>

class Database;

class ServerStartStop
{
private:
	int serverSocketFileDescriptor = -1;			///< Дескриптор слушающего сокета.
	sockaddr_in serverAddr{};
	static constexpr int reuseAddrOption = 1;		///< Значение для SO_REUSEADDR (1 — разрешить)
	
	void initServerAddr(const ServerConfig& config);
public:
	void start(const ServerConfig& config, Database* db);
	void stop();
};
