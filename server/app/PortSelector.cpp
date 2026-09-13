#include "PortSelector.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "Logger.h"
#include "ShutdownSignal.h"
bool isPortFree(int port)
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

int promptForPort(int defaultPort)
{
	std::string line;
	while (!ShutdownSignal::isRequested())
	{
		std::cout << "Input port (1..65535, Enter for default " << defaultPort << "): ";
		if (!std::getline(std::cin, line))
		{
			LOG_ERROR("Input error, exiting: {}", strerror(errno));
			exit(1);
		}
		if (line.empty())
		{
			if (isPortFree(defaultPort))
			{
				LOG_INFO("Selected default port {}", defaultPort);
				return defaultPort;
			}
			else
			{
				LOG_ERROR("Default port {} is already in use: {}", defaultPort, strerror(errno));
				continue;
			}
		}

		try
		{
			int port = std::stoi(line);
			if (port < 1 || port > 65535)
			{
				LOG_ERROR("Port {} out of range (1...65535): {}", port, strerror(errno));
				continue;
			}
			if (isPortFree(port))
			{
				LOG_INFO("Selected port {}", port);
				return port;
			}
			else
			{
				LOG_ERROR("Port {} is already in use: {}", port, strerror(errno));
			}
		}
		catch (const std::invalid_argument&)
		{
			LOG_ERROR("Invalid port number entered: '{}'", line);
		}
		catch (const std::out_of_range&)
		{
			LOG_ERROR("Port number out of integer range: '{}'", line);
		}
	}
	return -1;
}
