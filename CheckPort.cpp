#include "CheckPort.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "Logger.h"

bool tryCreateSocketOnPort(int port)
{
	int testSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (testSocket < 0)
		return false;
	constexpr int reuse = 1;
	setsockopt(testSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	sockaddr_in addr{};
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(port);

	bool isFree = (bind(testSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0);
	close(testSocket);
	return isFree;
}

int getValidPort(int defaultPort)
{
	std::string line;
	while (true)
	{
		std::cout << "Input port (1..65535, Enter for default " << defaultPort << "): ";
		if (!std::getline(std::cin, line))
		{
			Logger::instance().error("Input error, exiting: {}", strerror(errno));
			exit(1);
		}
		if (line.empty())
		{
			if (tryCreateSocketOnPort(defaultPort))
			{
				Logger::instance().info("Selected default port {}", defaultPort);
				return defaultPort;
			}
			else
			{
				Logger::instance().error("Default port {} is already in use: {}", defaultPort, strerror(errno));
				continue;
			}
		}

		try
		{
			int port = std::stoi(line);
			if (port < 1 || port > 65535)
			{
				Logger::instance().error("Port {} out of range (1...65535): {}", port, strerror(errno));
				continue;
			}
			if (tryCreateSocketOnPort(port))
			{
				Logger::instance().info("Selected port {}", port);
				return port;
			}
			else
			{
				Logger::instance().error("Port {} is already in use: {}", port, strerror(errno));
			}
		}
		catch (const std::invalid_argument&)
		{
			Logger::instance().error("Invalid port number entered: '{}'", line);
		}
		catch (const std::out_of_range&)
		{
			Logger::instance().error("Port number out of integer range: '{}'", line);
		}
	}
}
