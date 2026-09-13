#pragma once
#include <vector>
#include <cstdint>
#include "PacketData.h"

class ClientSession;
class AuthService;
class ChatService;
class MessageService;

class PacketDispatcher
{
public:
	PacketDispatcher(AuthService* authService,
					 ChatService* chatService,
					 MessageService* messageService);

	void dispatch(const PacketHeaderRaw& header,
				  const std::vector<uint8_t>& body,
				  ClientSession& session);

private:
	AuthService* authService_;
	ChatService* chatService_;
	MessageService* messageService_;

	void handleConnectRequest(const PacketHeaderRaw& header, ClientSession& session);
	void handleDisconnectRequest(ClientSession& session);
};
