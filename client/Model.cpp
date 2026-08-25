#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include <QApplication>
#include <QWidget>
#include "ClientConfig.h"
#include "Logger.h"
#include "Packets.h"
#include "Serializer.h"

sockaddr_in initServerAddr(const ClientConfig& config)
{
	sockaddr_in addr{};
	addr.sin_family			= config.getDomain();
	addr.sin_addr.s_addr	= htonl(config.getAddr());
	addr.sin_port			= htons(config.getPort());
	return addr;
}

int main(int argc, char* argv[])
{
	ClientConfig config;
	int clientSocketFd = socket(config.getDomain(), config.getType(), config.getProtocol());
	if (clientSocketFd < 0)
	{
		Logger::instance().critical("socket() failed: {}", strerror(errno));
		return 1;
	}
	sockaddr_in serverAddr = initServerAddr(config);
	if (connect(clientSocketFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		Logger::instance().error("Client connect error: {}", strerror(errno));
		return 1;
	}
	
	std::string username = "Chito Dristo";
	std::string password = "tralalala09";
	
	PacketHeaderRaw header;
	header.type = (uint16_t)PacketType::RegisterRequest;
	header.messageID = 1;
	header.sessionID = 1;
	header.messageLen = username.size() + password.size();
	
	RegisterRequestPacket packet;
	packet.username = username;
	packet.password = password;
	
	Serializer serializer;
	std::vector<uint8_t> message = serializer.buildRegisterRequestPacket(header.messageID, header.sessionID, packet);
	
	send(clientSocketFd, message.data(), message.size(), 0);
	
	
	
	close(clientSocketFd);
	Logger::instance().info("Client stopped");
	return 0;
}
