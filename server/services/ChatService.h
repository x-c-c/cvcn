#pragma once
#include <cstdint>
#include "PacketData.h"

class IClientSession;
class IUserRepository;
class IChatRepository;

class ChatService
{
public:
    ChatService(IUserRepository* userRepo, IChatRepository* chatRepo);

    void handleFindUserRequest(const PacketHeaderRaw& header,
                               const FindUserRequestData& data,
                               IClientSession* session);

    void handleCreateChatRequest(const PacketHeaderRaw& header,
                                 const CreateChatRequestData& data,
                                 IClientSession* session);

    void handleChatListRequest(const PacketHeaderRaw& header,
                               IClientSession* session);

private:
    IUserRepository* userRepository_;
    IChatRepository* chatRepository_;
};
