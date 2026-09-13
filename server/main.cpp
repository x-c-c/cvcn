#include "Logger.h"
#include "ServerConfig.h"
#include "CheckPort.h"
#include "ServerStartStop.h"
#include "SigintHandler.h"
#include "Database.h"
#include "UserRepository.h"
#include "ChatRepository.h"
#include "MessageRepository.h"
#include "Epoller.h"
#include "SessionManager.h"

int main()
{
	SigintHandler::setup();
	Logger::instance().info("Server starting up");

	Database db("chat.db");
	UserRepository userRepo(db.getHandle());
	ChatRepository chatRepo(db.getHandle());
	MessageRepository msgRepo(db.getHandle());

	ServerConfig config;
	const int chosenPort = getValidPort(config.getPort());
	if (chosenPort == -1)
	{
		Logger::instance().info("Shutdown requested during port selection");
		return 0;
	}
	config.setPort(chosenPort);

	ServerStartStop server;
	server.start(config);

	Epoller epoller;
	SessionManager sessionManager(&userRepo, &chatRepo, &msgRepo, &epoller);

	epoller.setNewConnectionCallback([&sessionManager](int fd){ sessionManager.onNewConnection(fd); });
	epoller.setReadEventCallback   ([&sessionManager](int fd){ sessionManager.onRead(fd); });
	epoller.setWriteEventCallback  ([&sessionManager](int fd){ sessionManager.onWrite(fd); });
	epoller.setErrorEventCallback  ([&sessionManager](int fd, uint32_t ev){ sessionManager.onError(fd, ev); });

	epoller.startEpollLoop(server.getServerSocketFD());

	Logger::instance().info("Server shutdown");
	return 0;
}
