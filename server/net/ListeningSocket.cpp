/**
 * @file    ListeningSocket.cpp
 * @brief   Реализация ListeningSocket.
 * @see ListeningSocket.h
 */

#include "./ListeningSocket.h"
#include "../utils/Logger.h"
#include <cstring>
#include <stdexcept>
#include <string>

ListeningSocket::~ListeningSocket()
{
    closeSocket();
}

void ListeningSocket::initServerAddr(const ServerConfig& config)
{
    serverAddr.sin_family      = config.getDomain();
    serverAddr.sin_addr.s_addr = config.getAddr();       // уже в network order
    serverAddr.sin_port        = htons(config.getPort()); // host → network
}

void ListeningSocket::start(const ServerConfig& config)
{
    // Если сокет уже открыт — переоткрываем.
    if (serverSocketFD_ != -1)
    {
        Logger::instance().warn("Server socket already open, closing it first");
        closeSocket();
    }

    //  1. Создание сокета 
    serverSocketFD_ = socket(config.getDomain(), config.getType(), config.getProtocol());
    if (serverSocketFD_ < 0)
    {
        closeSocket();
        throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));
    }

    //  2. SO_REUSEADDR 
    // Разрешает bind на порт, который недавно был занят (TIME_WAIT).
    // Без этого после перезапуска сервера bind падает с EADDRINUSE
    // в течение ~60 секунд.
    setsockopt(serverSocketFD_, SOL_SOCKET, SO_REUSEADDR,
               &reuseAddrOption, sizeof(reuseAddrOption));

    //  3. bind 
    initServerAddr(config);
    if (bind(serverSocketFD_, reinterpret_cast<sockaddr*>(&serverAddr),
             sizeof(serverAddr)) != 0)
    {
        closeSocket();
        throw std::runtime_error(std::string("bind() failed ") + strerror(errno));
    }

    //  4. listen 
    // SOMAXCONN — максимальная длина очереди входящих соединений,
    // которую поддерживает ядро. Для большинства задач достаточно.
    if (listen(serverSocketFD_, SOMAXCONN) != 0)
    {
        closeSocket();
        throw std::runtime_error(std::string("listen() failed ") + strerror(errno));
    }

    Logger::instance().info("Server listening on port {}", config.getPort());
}

void ListeningSocket::closeSocket()
{
    if (serverSocketFD_ != -1)
    {
        close(serverSocketFD_);
        serverSocketFD_ = -1;   // защита от повторного close
    }
    Logger::instance().info("Closing server socket");
}
