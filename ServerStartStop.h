/**
 * @file ServerStartStop.h
 * @brief Управление запуском и остановкой сервера.
 */

#pragma once
#include "ServerConfig.h"
#include <sys/socket.h>
#include <unistd.h>
#include "Epoller.h"
class Database;
class ServerStartStop
{
private:
	int serverSocketFileDescriptor = -1;   ///< Дескриптор слушающего сокета.
	sockaddr_in serverAddr{};
	static constexpr int reuseAddrOption = 1;	///< Значение для SO_REUSEADDR (1 — разрешить)
	static Epoller* currentEpoller_;
	
	void initServerAddr(const ServerConfig& config);
public:
	/**
	 * @brief Создаёт сокет, привязывает к адресу, слушает и запускает цикл epoll.
	 * @param config конфигурация сервера
	 */
	void start(const ServerConfig& config, Database* db);
	/**
	 * @brief Заглушка для будущей мягкой остановки.
	 */
	void stop();
	static void handleSignal(int signum);

};
