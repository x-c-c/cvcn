#include "MessageService.h"
#include "ClientSession.h"
#include "MessageRepository.h"
#include "ChatRepository.h"
#include "SessionRegistry.h"
#include "PacketBuilder.h"
#include "Logger.h"

MessageService::MessageService(MessageRepository* msgRepo,
							   ChatRepository* chatRepo,
							   SessionRegistry* sessionRegistry):
	msgRepo_(msgRepo),
	chatRepo_(chatRepo),
	sessionRegistry_(sessionRegistry){}

void MessageService::handleMessageSend(const PacketHeaderRaw& header,
									   const MessageSendData& data,
									   ClientSession& session)
{
	(void)header;

	const int senderID = session.getUserID();
	if (senderID == -1)
	{
		Logger::instance().warn("MessageSend before auth (fd {})", session.getfileDescriptor());
		return;
	}

	if (!msgRepo_->saveMessage(data.chatID, senderID, data.text))
	{
		Logger::instance().error("Failed to save message from '{}' (uid {}) in chat {}",
			session.getUsername(), senderID, data.chatID);
	}

	MessageReceiveData payload;
	payload.senderID       = static_cast<uint32_t>(senderID);
	payload.senderUsername = session.getUsername();
	payload.chatID         = data.chatID;
	payload.text           = data.text;

	const std::vector<int> members = chatRepo_->getChatMemberIDs(data.chatID);
	broadcastToChat(members, senderID, payload);

	Logger::instance().info("Message from '{}' (uid {}) to chat {}: '{}' (recipients: {})",
		session.getUsername(), senderID, data.chatID, data.text, members.size() - 1);
}

void MessageService::broadcastToChat(const std::vector<int>& memberIDs,
									 int excludeUserID,
									 const MessageReceiveData& payload)
{
	if (!sessionRegistry_)
		return;

	// Сервер выступает инициатором, messageID и sessionID = 0.
	// Если позже понадобится подтверждение доставки, здесь появится реальный messageID.
	const std::vector<uint8_t> packet = PacketBuilder::buildPacket(0, 0, payload);

	for (int uid : memberIDs)
	{
		if (uid == excludeUserID)
			continue;

		ClientSession* recipient = sessionRegistry_->findByUserID(uid);
		if (recipient == nullptr)
			continue;   // получатель не в сети, сообщение останется в БД

		recipient->sendRaw(packet);
		Logger::instance().info("MessageReceive pushed to '{}' (uid {})",
			recipient->getUsername(), uid);
	}
}
