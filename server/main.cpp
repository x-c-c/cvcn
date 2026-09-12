#include "Logger.h"
#include "ServerConfig.h"
#include "CheckPort.h"
#include "ServerStartStop.h"
#include "SigintHandler.h"
#include "Database.h"
#include "Epoller.h"
#include "SessionManager.h"
int main()
{
	SigintHandler::setup();
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
	ServerStartStop server;
	server.start(config);
	
	Epoller epoller;
	SessionManager sessionManager(&db, &epoller);

	// это ужасно выглядит
	epoller.setNewConnectionCallback
	(
		[&sessionManager](int fileDescriptor)
		{
			sessionManager.onNewConnection(fileDescriptor);
		}
	);
	epoller.setReadEventCallback	([&sessionManager](int fileDescriptor){ sessionManager.onRead(fileDescriptor); });
	epoller.setWriteEventCallback	([&sessionManager](int fileDescriptor){ sessionManager.onWrite(fileDescriptor); });
	epoller.setErrorEventCallback	([&sessionManager](int fileDescriptor, uint32_t ev){ sessionManager.onError(fileDescriptor, ev); });
	
	epoller.startEpollLoop(server.getServerSocketFD());
	
	
	Logger::instance().info("Server shutdown");
	return 0;
}
