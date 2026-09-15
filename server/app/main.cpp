#include "../utils/Logger.h"
#include "../config/ServerConfig.h"
#include "../net/PortSelector.h"
#include "../net/ListeningSocket.h"
#include "../net/EventPoller.h"
#include "../net/SessionManager.h"
#include "../storage/Database.h"
#include "./ShutdownSignal.h"

int main()
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
	
	
	Logger::instance().info("Server shutdown");
	return 0;
}
