#pragma once
#include <cstdint>
#include "PacketData.h"

class ClientSession;
class UserRepository;
class ChatRepository;

class ChatService
{
public:
	ChatService(UserRepository* userRepo, ChatRepository* chatRepo);

	void handleFindUserRequest(const PacketHeaderRaw& header,
							   const FindUserRequestData& data,
							   ClientSession& session);

	void handleCreateChatRequest(const PacketHeaderRaw& header,
								 const CreateChatRequestData& data,
								 ClientSession& session);

	void handleChatListRequest(const PacketHeaderRaw& header,
							   ClientSession& session);

private:
	UserRepository* userRepo_;
	ChatRepository* chatRepo_;
};
