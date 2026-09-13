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
	LOG_INFO("Server starting up");

	Database db("chat.db");
	UserRepository userRepository(db.getHandle());
	ChatRepository chatRepository(db.getHandle());
	MessageRepository messageRepository(db.getHandle());

	SessionRegistry sessionRegistry;
	AuthService authService(&userRepository, &sessionRegistry);
	ChatService chatService(&userRepository, &chatRepository);
	MessageService messageService(&messageRepository, &chatRepository, &sessionRegistry);

	PacketDispatcher dispatcher(&authService, &chatService, &messageService);

	ServerConfig config;
	const int chosenPort = promptForPort(config.getPort());
	if (chosenPort == -1)
	{
		LOG_INFO("Shutdown requested during port selection");
		return 0;
	}
	config.setPort(chosenPort);

	ListeningSocket listener;
	listener.startListening(config);

	EventPoller eventPoller;
	SessionManager sessionManager(&dispatcher, &sessionRegistry, &eventPoller);

	eventPoller.setNewConnectionCallback([&sessionManager](int fd){ sessionManager.onNewConnection(fd); });
	eventPoller.setReadEventCallback   ([&sessionManager](int fd){ sessionManager.onRead(fd); });
	eventPoller.setWriteEventCallback  ([&sessionManager](int fd){ sessionManager.onWrite(fd); });
	eventPoller.setErrorEventCallback  ([&sessionManager](int fd, uint32_t ev){ sessionManager.onError(fd, ev); });

	eventPoller.startEventLoop(listener.fileDescriptor());

	LOG_INFO("Server shutdown");
	return 0;
}
