/**
 * @file ServerStartStop.h
 * @brief Управление запуском и остановкой сервера.
 */

#pragma once
#include "ServerConfig.h"
#include <sys/socket.h>
#include <unistd.h>

class ServerStartStop
{
public:
	/**
	 * @brief Создаёт сокет, привязывает к адресу, слушает и запускает цикл epoll.
	 * @param config конфигурация сервера
	 */
	void start(const ServerConfig& config);

	/**
	 * @brief Заглушка для будущей мягкой остановки.
	 */
	void stop();

private:
	int serverSocketFileDescriptor = -1;   ///< Дескриптор слушающего сокета.
};
