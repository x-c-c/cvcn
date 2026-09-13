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

	// Пока только сохраняем и логируем. Рассылку включим, когда
	// на клиенте появится обработка MessageReceive.
	const bool saved = msgRepo_->saveMessage(data.chatID, senderID, data.text);
	if (!saved)
	{
		Logger::instance().error("Failed to save message from '{}' (uid {}) in chat {}",
			session.getUsername(), senderID, data.chatID);
	}

	Logger::instance().info("Message from '{}' (uid {}) to chat {}: '{}'",
		session.getUsername(), senderID, data.chatID, data.text);
}
