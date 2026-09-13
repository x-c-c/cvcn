#include "PacketDispatcher.h"
#include "IClientSession.h"
#include "AuthService.h"
#include "ChatService.h"
#include "MessageService.h"
#include "PacketBuilder.h"
#include "PacketParser.h"
#include "Validator.h"
#include "Logger.h"
#include <exception>

PacketDispatcher::PacketDispatcher(AuthService* authService,
                                   ChatService* chatService,
                                   MessageService* messageService):
    authService_(authService),
    chatService_(chatService),
    messageService_(messageService){}

void PacketDispatcher::dispatch(const PacketHeaderRaw& header,
                                const std::vector<uint8_t>& body,
                                IClientSession* session)
{
    try
    {
        switch (static_cast<PacketType>(header.type))
        {
        case PacketType::ConnectRequest:
            handleConnectRequest(header, session);
            break;

        case PacketType::DisconnectRequest:
            handleDisconnectRequest(session);
            break;

        case PacketType::RegisterRequest:
        {
            RegisterRequestData data;
            if (!PacketParser::parseData(body, data))
            {
                LOG_WARN("Bad RegisterRequest from fd {}", session->getFileDescriptor());
                break;
            }
            if (!Validator::validateUsername(data.username) || !Validator::validatePassword(data.password))
            {
                LOG_WARN("RegisterRequest rejected: invalid format (fd {})", session->getFileDescriptor());
                break;
            }
            authService_->handleRegisterRequest(header, data, session);
            break;
        }

        case PacketType::AuthRequest:
        {
            AuthRequestData data;
            if (!PacketParser::parseData(body, data))
            {
                LOG_WARN("Bad AuthRequest from fd {}", session->getFileDescriptor());
                break;
            }
            if (!Validator::validateUsername(data.username) || !Validator::validatePassword(data.password))
            {
                LOG_WARN("AuthRequest rejected: invalid format (fd {})", session->getFileDescriptor());
                break;
            }
            authService_->handleAuthRequest(header, data, session);
            break;
        }

        case PacketType::DeleteRequest:
        {
            DeleteRequestData data;
            if (!PacketParser::parseData(body, data))
            {
                LOG_WARN("Bad DeleteRequest from fd {}", session->getFileDescriptor());
                break;
            }
            if (!Validator::validateUsername(data.username) || !Validator::validatePassword(data.password))
            {
                LOG_WARN("DeleteRequest rejected: invalid format (fd {})", session->getFileDescriptor());
                break;
            }
            authService_->handleDeleteRequest(header, data, session);
            break;
        }

        case PacketType::FindUserRequest:
        {
            FindUserRequestData data;
            if (!PacketParser::parseData(body, data))
            {
                LOG_WARN("Bad FindUserRequest from fd {}", session->getFileDescriptor());
                break;
            }
            if (data.query.empty() || data.query.size() > Validator::MAX_USERNAME_LENGTH)
            {
                LOG_WARN("FindUserRequest rejected: bad query (fd {})", session->getFileDescriptor());
                break;
            }
            chatService_->handleFindUserRequest(header, data, session);
            break;
        }

        case PacketType::CreateChatRequest:
        {
            CreateChatRequestData data;
            if (!PacketParser::parseData(body, data))
            {
                LOG_WARN("Bad CreateChatRequest from fd {}", session->getFileDescriptor());
                break;
            }
            if (!Validator::validateUsername(data.peerUsername))
            {
                LOG_WARN("CreateChatRequest rejected: bad peer (fd {})", session->getFileDescriptor());
                break;
            }
            chatService_->handleCreateChatRequest(header, data, session);
            break;
        }

        case PacketType::ChatListRequest:
            chatService_->handleChatListRequest(header, session);
            break;

        case PacketType::MessageSend:
        {
            MessageSendData data;
            if (!PacketParser::parseData(body, data))
            {
                LOG_WARN("Bad MessageSend from fd {}", session->getFileDescriptor());
                break;
            }
            if (!Validator::validateChatID(data.chatID) || !Validator::validateMessage(data.text))
            {
                LOG_WARN("MessageSend rejected: invalid format (fd {})", session->getFileDescriptor());
                break;
            }
            messageService_->handleMessageSend(header, data, session);
            break;
        }

        default:
            LOG_WARN("Unknown packet type 0x{:X} from fd {}",
                header.type, session->getFileDescriptor());
            break;
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception in dispatch (fd {}): {}", session->getFileDescriptor(), e.what());
    }
    catch (...)
    {
        LOG_ERROR("Unknown exception in dispatch (fd {})", session->getFileDescriptor());
    }
}

void PacketDispatcher::handleConnectRequest(const PacketHeaderRaw& header, IClientSession* session)
{
    const uint32_t newSessionID = header.sessionID
        ? header.sessionID
        : static_cast<uint32_t>(session->getFileDescriptor());

    ConnectResponseData resp;
    session->sendRaw(PacketBuilder::buildPacket(header.messageID, newSessionID, resp));
    LOG_INFO("ConnectResponse sent to fd {} (sessionID={})",
        session->getFileDescriptor(), newSessionID);
}

void PacketDispatcher::handleDisconnectRequest(IClientSession* session)
{
    LOG_INFO("Client {} requested disconnect", session->getFileDescriptor());
    session->closeSession();
}
