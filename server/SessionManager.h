#pragma once
#include <unordered_map>
#include <memory>
#include <cstdint>

class ClientSession;
class UserRepository;
class ChatRepository;
class MessageRepository;
class SessionRegistry;
class Epoller;

class SessionManager
{
public:
	SessionManager(UserRepository* userRepo,
				   ChatRepository* chatRepo,
				   MessageRepository* msgRepo,
				   SessionRegistry* sessionRegistry,
				   Epoller* epoller);
	~SessionManager();

	void onNewConnection(int fileDescriptor);
	void onRead(int fileDescriptor);
	void onWrite(int fileDescriptor);
	void onError(int fileDescriptor, uint32_t events);

private:
	UserRepository* userRepo_;
	ChatRepository* chatRepo_;
	MessageRepository* msgRepo_;
	SessionRegistry* sessionRegistry_;
	Epoller* epoller_;
	std::unordered_map<int, std::unique_ptr<ClientSession>> sessions_;

	void closeClient(int fileDescriptor);
};
