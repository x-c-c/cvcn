#pragma once
#include <vector>
#include <cstdint>
#include "PacketData.h"

class IClientSession;
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
                  IClientSession* session);

private:
    AuthService* authService_;
    ChatService* chatService_;
    MessageService* messageService_;

    void handleConnectRequest(const PacketHeaderRaw& header, IClientSession* session);
    void handleDisconnectRequest(IClientSession* session);
};
