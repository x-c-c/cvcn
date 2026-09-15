/**
 * @file    PortSelector.cpp
 * @brief   Реализация проверки и выбора порта.
 *
 * @details
 *   tryCreateSocketOnPort — «попробовать и забыть»: создаёт сокет,
 *   пытается bind, закрывает.
 *
 *   getValidPort — цикл диалога с пользователем. Каждая итерация
 *   завершается либо успешным возвратом порта, либо сообщением
 *   об ошибке и повторным приглашением. Выход — только по
 *   ShutdownSignal (вернёт -1) или по успеху.
 *
 * @see PortSelector.h
 */

#include "./PortSelector.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../utils/Logger.h"
#include "../app/ShutdownSignal.h"

bool tryCreateSocketOnPort(int port)
{
    // AF_INET + SOCK_STREAM — TCP-сокет. Протокол 0 означает
    // «выбрать по умолчанию для типа» (IPPROTO_TCP).
    const int testSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (testSocket < 0)
        return false;   // нехватка fd или запрет на сокеты

    // SO_REUSEADDR на тестовом сокете нужен, чтобы не получить
    // ложное «занято» из-за TIME_WAIT после недавнего закрытия
    // реального слушающего сокета.
    constexpr int reuse = 1;
    setsockopt(testSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    // bind — единственный вызов, результат которого нас интересует.
    // Успешный bind значит: порт свободен (в очереди на INADDR_ANY
    // нет другого слушающего сокета). Немедленно закрываем.
    const bool isFree = (bind(testSocket,
                              reinterpret_cast<sockaddr*>(&addr),
                              sizeof(addr)) == 0);
    close(testSocket);
    return isFree;
}

int getValidPort(int defaultPort)
{
    std::string line;

    // Крутимся до успешного выбора. Ctrl+C выставит флаг через
    // ShutdownSignal::handler, и цикл завершится с -1.
    while (!ShutdownSignal::isStopRequested())
    {
        std::cout << "Input port (1..65535, Enter for default "
                  << defaultPort << "): ";

        if (!std::getline(std::cin, line))
        {
            Logger::instance().error("Input error, exiting: {}", strerror(errno));
            exit(1);
        }

        //  Пустой ввод — берём defaultPort
        if (line.empty())
        {
            if (tryCreateSocketOnPort(defaultPort))
            {
                Logger::instance().info("Selected default port {}", defaultPort);
                return defaultPort;
            }
            else
            {
                Logger::instance().error(
                    "Default port {} is already in use: {}",
                    defaultPort, strerror(errno));
                continue;
            }
        }

        // Разбор введённой строки 
        try
        {
            const int port = std::stoi(line);

            if (port < 1 || port > 65535)
            {
                Logger::instance().error(
                    "Port {} out of range (1...65535): {}", port, strerror(errno));
                continue;
            }

            if (tryCreateSocketOnPort(port))
            {
                Logger::instance().info("Selected port {}", port);
                return port;
            }
            else
            {
                Logger::instance().error(
                    "Port {} is already in use: {}", port, strerror(errno));
            }
        }
        catch (const std::invalid_argument&)
        {
            // Строка не число вообще (например, "abc", "", "12x").
            Logger::instance().error("Invalid port number entered: '{}'", line);
        }
        catch (const std::out_of_range&)
        {
            // Число синтаксически корректно, но не влезает в int
            // (например, "99999999999999999999").
            Logger::instance().error( "Port number out of integer range: '{}'", line);
        }
    }
    return -1;
}
