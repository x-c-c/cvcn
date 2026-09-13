#pragma once
#include <cstdint>
#include "PacketData.h"

class ClientSession;
class ChatRepository;
class MessageRepository;
class SessionRegistry;

class MessageService
{
public:
	MessageService(MessageRepository* msgRepo,
				   ChatRepository* chatRepo,
				   SessionRegistry* sessionRegistry);

	void handleMessageSend(const PacketHeaderRaw& header,
						   const MessageSendData& data,
						   ClientSession& session);

private:
	MessageRepository* msgRepo_;
	ChatRepository* chatRepo_;
	SessionRegistry* sessionRegistry_;
};
