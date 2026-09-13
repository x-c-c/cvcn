#pragma once
#include <cstdint>
#include "PacketData.h"

class IClientSession;
class IChatRepository;
class IMessageRepository;
class ISessionRegistry;

class MessageService
{
public:
    MessageService(IMessageRepository* msgRepo,
                   IChatRepository* chatRepo,
                   ISessionRegistry* sessionRegistry);

    void handleMessageSend(const PacketHeaderRaw& header,
                           const MessageSendData& data,
                           IClientSession* session);

private:
    IMessageRepository* messageRepository_;
    IChatRepository* chatRepository_;
    ISessionRegistry* sessionRegistry_;

    void broadcastToChat(const std::vector<int>& memberIDs,
                         int excludeUserID,
                         const MessageReceiveData& payload);
};
