#pragma once
#include "ServerConfig.h"
#include <sys/socket.h>
#include <unistd.h>

class Database;

class ListeningSocket
{
private:
	int serverSocketFD_ = -1;			///< Дескриптор слушающего сокета.
	sockaddr_in serverAddr{};
	static constexpr int reuseAddrOption = 1;		///< Значение для SO_REUSEADDR (1 — разрешить)
	
	void initServerAddr(const ServerConfig& config);
public:
	~ListeningSocket();
	int fileDescriptor()	{ return serverSocketFD_; }
	void listen(const ServerConfig& config);
	void closeSocket();
};
