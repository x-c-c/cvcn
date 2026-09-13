#pragma once
#include <unordered_map>
#include <memory>
#include <cstdint>

class ClientSession;
class PacketDispatcher;
class SessionRegistry;
class Epoller;

class SessionManager
{
public:
	SessionManager(PacketDispatcher* dispatcher,
				   SessionRegistry* sessionRegistry,
				   Epoller* epoller);
	~SessionManager();

	void onNewConnection(int fileDescriptor);
	void onRead(int fileDescriptor);
	void onWrite(int fileDescriptor);
	void onError(int fileDescriptor, uint32_t events);

private:
	PacketDispatcher* dispatcher_;
	SessionRegistry* sessionRegistry_;
	Epoller* epoller_;
	std::unordered_map<int, std::unique_ptr<ClientSession>> sessions_;

	void closeClient(int fileDescriptor);
};
