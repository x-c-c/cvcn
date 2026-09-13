#include "MessageService.h"
#include "IClientSession.h"
#include "IMessageRepository.h"
#include "IChatRepository.h"
#include "ISessionRegistry.h"
#include "PacketBuilder.h"
#include "Logger.h"

MessageService::MessageService(IMessageRepository* msgRepo,
                               IChatRepository* chatRepo,
                               ISessionRegistry* sessionRegistry):
    messageRepository_(msgRepo),
    chatRepository_(chatRepo),
    sessionRegistry_(sessionRegistry){}

void MessageService::handleMessageSend(const PacketHeaderRaw& header,
                                       const MessageSendData& data,
                                       IClientSession* session)
{
    (void)header;

    const int senderID = session->getUserID();
    if (senderID == -1)
    {
        LOG_WARN("MessageSend before auth (fd {})", session->getFileDescriptor());
        return;
    }

    if (!messageRepository_->saveMessage(data.chatID, senderID, data.text))
    {
        LOG_ERROR("Failed to save message from '{}' (userID {}) in chat {}",
            session->getUsername(), senderID, data.chatID);
    }

    MessageReceiveData payload;
    payload.senderUsername = session->getUsername();
    payload.chatID         = data.chatID;
    payload.text           = data.text;

    const std::vector<int> members = chatRepository_->getChatMemberIDs(data.chatID);
    broadcastToChat(members, senderID, payload);

    LOG_INFO("Message from '{}' (userID {}) to chat {}: '{}' (recipients: {})",
        session->getUsername(), senderID, data.chatID, data.text, members.size() - 1);
}

void MessageService::broadcastToChat(const std::vector<int>& memberIDs,
                                     int excludeUserID,
                                     const MessageReceiveData& payload)
{
    if (!sessionRegistry_)
        return;

    const std::vector<uint8_t> packet = PacketBuilder::buildPacket(0, 0, payload);

    for (int userID : memberIDs)
    {
        if (userID == excludeUserID)
            continue;

        IClientSession* recipient = sessionRegistry_->findByUserID(userID);
        if (recipient == nullptr)
            continue;

        recipient->sendRaw(packet);
        LOG_INFO("MessageReceive pushed to '{}' (userID {})",
            recipient->getUsername(), userID);
    }
}
