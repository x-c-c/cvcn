#include "Logger.h"
#include "ServerConfig.h"
#include "PortSelector.h"
#include "ListeningSocket.h"
#include "ShutdownSignal.h"
#include "Database.h"
#include "UserRepository.h"
#include "ChatRepository.h"
#include "MessageRepository.h"
#include "SessionRegistry.h"
#include "AuthService.h"
#include "ChatService.h"
#include "MessageService.h"
#include "PacketDispatcher.h"
#include "EventPoller.h"
#include "SessionManager.h"

int main()
{
	ShutdownSignal::setup();
	Logger::instance().info("Server starting up");

	Database db("chat.db");
	UserRepository userRepo(db.getHandle());
	ChatRepository chatRepo(db.getHandle());
	MessageRepository msgRepo(db.getHandle());

	SessionRegistry sessionRegistry;
	AuthService authService(&userRepo, &sessionRegistry);
	ChatService chatService(&userRepo, &chatRepo);
	MessageService messageService(&msgRepo, &chatRepo, &sessionRegistry);

	PacketDispatcher dispatcher(&authService, &chatService, &messageService);

	ServerConfig config;
	const int chosenPort = promptForPort(config.getPort());
	if (chosenPort == -1)
	{
		Logger::instance().info("Shutdown requested during port selection");
		return 0;
	}
	config.setPort(chosenPort);

	ListeningSocket listener;
	listener.listen(config);

	EventPoller epoller;
	SessionManager sessionManager(&dispatcher, &sessionRegistry, &epoller);

	epoller.setNewConnectionCallback([&sessionManager](int fd){ sessionManager.onNewConnection(fd); });
	epoller.setReadEventCallback   ([&sessionManager](int fd){ sessionManager.onRead(fd); });
	epoller.setWriteEventCallback  ([&sessionManager](int fd){ sessionManager.onWrite(fd); });
	epoller.setErrorEventCallback  ([&sessionManager](int fd, uint32_t ev){ sessionManager.onError(fd, ev); });

	epoller.startEpollLoop(listener.fileDescriptor());

	Logger::instance().info("Server shutdown");
	return 0;
}
