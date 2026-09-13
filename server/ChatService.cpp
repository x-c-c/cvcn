#include "ChatService.h"
#include "ClientSession.h"
#include "UserRepository.h"
#include "ChatRepository.h"
#include "PacketBuilder.h"
#include "Logger.h"

ChatService::ChatService(UserRepository* userRepo, ChatRepository* chatRepo):
	userRepo_(userRepo),
	chatRepo_(chatRepo){}

void ChatService::handleFindUserRequest(const PacketHeaderRaw& header,
										const FindUserRequestData& data,
										ClientSession& session)
{
	const int userID = session.getUserID();
	if (userID == -1)
	{
		Logger::instance().warn("FindUserRequest before auth (fd {})", session.getfileDescriptor());
		FindUserResponseData resp;
		session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
		return;
	}

	FindUserResponseData resp;
	resp.usernames = chatRepo_->findUsers(data.query, userID);
	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
	Logger::instance().info("FindUser '{}' from fd {} → {} results",
		data.query, session.getfileDescriptor(), resp.usernames.size());
}

void ChatService::handleCreateChatRequest(const PacketHeaderRaw& header,
										  const CreateChatRequestData& data,
										  ClientSession& session)
{
	const int userID = session.getUserID();
	CreateChatResponseData resp{};

	if (userID == -1)
	{
		resp.success = 0;
	}
	else
	{
		const int peerID = userRepo_->getUserID(data.peerUsername);
		if (peerID == -1 || peerID == userID)
		{
			resp.success = 0;
			Logger::instance().warn("CreateChat failed for '{}': peer not found (fd {})",
				data.peerUsername, session.getfileDescriptor());
		}
		else
		{
			const int chatID = chatRepo_->findOrCreateDirectChat(userID, peerID);
			if (chatID > 0)
			{
				resp.success = 1;
				resp.chatID = static_cast<uint32_t>(chatID);
				resp.peerUsername = data.peerUsername;
				Logger::instance().info("Chat {} created/found between '{}' and '{}'",
					chatID, session.getUsername(), data.peerUsername);
			}
			else
			{
				resp.success = 0;
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
		resp.chats = chatRepo_->getUserChats(userID);
	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}
