/**
 * @file    main.cpp
 * @brief   Точка входа сервера.
 *
 * @details
 *   Порядок инициализации:
 *     1. ShutdownSignal  — обработчики SIGINT/SIGTERM, чтобы Ctrl+C корректно останавливал epoll-цикл.
 *     2. Logger          — единая точка логирования.
 *     3. Database        — открывает chat.db.
 *     4. PortSelector    — интерактивный выбор свободного порта.
 *     5. ListeningSocket — создание серверного слушающего сокета (socket/bind/listen).
 *     6. EventPoller     — epoll-цикл для отслеживания изменений на сокетах клиентов.
 *     7. SessionManager  — владеет ClientSession-ами, связан с EventPoller через callbacks.
 *
 * @note  Сервер однопоточный: весь event-loop крутится в этом потоке.
 * @see   ShutdownSignal, ListeningSocket, EventPoller, SessionManager
 */
 
#include "../utils/Logger.h"
#include "../config/ServerConfig.h"
#include "../net/PortSelector.h"
#include "../net/ListeningSocket.h"
#include "../net/EventPoller.h"
#include "../net/SessionManager.h"
#include "../storage/Database.h"
#include "./ShutdownSignal.h"

#include <exception>
#include <cstdlib>
#include <iostream>

int main()
{
	try
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
		if (server.getServerSocketFD() < 0)
		{
			Logger::instance().critical("Listening socket not available, exiting");
			return 1;
		}
			
		EventPoller epoller;
		SessionManager sessions(epoller, db);
		epoller.setNewConnectionCallback(
			[&sessions](int fd){ sessions.onNewConnection(fd); });
		epoller.setReadEventCallback(
			[&sessions](int fd) { sessions.onRead(fd); });
		epoller.setWriteEventCallback(
			[&sessions](int fd) { sessions.onWrite(fd); });
		epoller.setErrorEventCallback(
			[&sessions](int fd, uint32_t ev) { sessions.onError(fd, ev); });
		
		
		
		// throw std::runtime_error("test exception");		// просто проверить что вообще работают исключения
		
		
		epoller.startEpollLoop(server.getServerSocketFD());
		
		Logger::instance().info("Server shutdown");
	}
	catch (const std::exception& e)
    {
		Logger::instance().critical("Fatal: {}", e.what());
        return 1;
    }
    catch (...)
    {
        std::cerr << "[Fatal]: unknown exception" << std::endl;
        return 1;
    }
	return 0;
}
