#include "CheckPort.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

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
			std::cerr << "Input error, exiting" << std::endl;
			exit(1);
		}
		if (line.empty())
		{
			if (tryCreateSocketOnPort(defaultPort))
			{
				std::cout << "Server will start on port: " << defaultPort << std::endl;
				return defaultPort;
			}
			else
			{
				std::cerr << "Port " << defaultPort << " is already in use" << std::endl;
				continue;
			}
		}

		try
		{
			int port = std::stoi(line);
			if (port < 1 || port > 65535)
			{
				std::cerr << "Port out of range (1...65535)" << std::endl;
				continue;
			}
			if (tryCreateSocketOnPort(port))
			{
				std::cout << "Server start on port: " << port << std::endl;
				return port;
			}
			else
			{
				std::cerr << "Port " << port << " is already in use" << std::endl;
			}
		}
		catch (const std::invalid_argument&)
		{
			std::cerr << "Not a valid number" << std::endl;
		}
		catch (const std::out_of_range&)
		{
			std::cerr << "Number out of range" << std::endl;
		}
	}
}
