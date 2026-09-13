#include "ChatService.h"
#include "ClientSession.h"
#include "UserRepository.h"
#include "ChatRepository.h"
#include "PacketBuilder.h"
#include "Logger.h"

ChatService::ChatService(UserRepository* userRepo, ChatRepository* chatRepo):
	userRepository_(userRepo),
	chatRepository_(chatRepo){}

void ChatService::handleFindUserRequest(const PacketHeaderRaw& header,
										const FindUserRequestData& data,
										ClientSession& session)
{
	const int userID = session.getUserID();
	if (userID == -1)
	{
		LOG_WARN("FindUserRequest before auth (fd {})", session.getFileDescriptor());
		FindUserResponseData resp;
		session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
		return;
	}

	FindUserResponseData resp;
	resp.usernames = chatRepository_->findUsers(data.query, userID);
	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
	LOG_INFO("FindUser '{}' from fd {} → {} results",
		data.query, session.getFileDescriptor(), resp.usernames.size());
}

void ChatService::handleCreateChatRequest(const PacketHeaderRaw& header,
										  const CreateChatRequestData& data,
										  ClientSession& session)
{
	const int userID = session.getUserID();
	CreateChatResponseData resp{};

	if (userID == -1)
	{
		resp.success = false;
	}
	else
	{
		const int peerID = userRepository_->getUserID(data.peerUsername);
		if (peerID == -1 || peerID == userID)
		{
			resp.success = false;
			LOG_WARN("CreateChat failed for '{}': peer not found (fd {})",
				data.peerUsername, session.getFileDescriptor());
		}
		else
		{
			const int chatID = chatRepository_->findOrCreateDirectChat(userID, peerID);
			if (chatID > 0)
			{
				resp.success = true;
				resp.chatID = static_cast<uint32_t>(chatID);
				resp.peerUsername = data.peerUsername;
				LOG_INFO("Chat {} created/found between '{}' and '{}'",
					chatID, session.getUsername(), data.peerUsername);
			}
			else
			{
				resp.success = false;
			}
		}
	}

	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}

void ChatService::handleChatListRequest(const PacketHeaderRaw& header, ClientSession& session)
{
	ChatListResponseData resp;
	const int userID = session.getUserID();
	if (userID != -1)
		resp.chats = chatRepository_->getUserChats(userID);
	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}
