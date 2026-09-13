#include "PacketDispatcher.h"
#include "ClientSession.h"
#include "AuthService.h"
#include "ChatService.h"
#include "MessageService.h"
#include "PacketBuilder.h"
#include "PacketParser.h"
#include "Validator.h"
#include "Logger.h"

PacketDispatcher::PacketDispatcher(AuthService* authService,
								   ChatService* chatService,
								   MessageService* messageService):
	authService_(authService),
	chatService_(chatService),
	messageService_(messageService){}

void PacketDispatcher::dispatch(const PacketHeaderRaw& header,
								const std::vector<uint8_t>& body,
								ClientSession& session)
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
			Logger::instance().warn("Bad RegisterRequest from fd {}", session.getFileDescriptor());
			break;
		}
		if (!Validator::validateUsername(data.username) || !Validator::validatePassword(data.password))
		{
			Logger::instance().warn("RegisterRequest rejected: invalid format (fd {})", session.getFileDescriptor());
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
			Logger::instance().warn("Bad AuthRequest from fd {}", session.getFileDescriptor());
			break;
		}
		if (!Validator::validateUsername(data.username) || !Validator::validatePassword(data.password))
		{
			Logger::instance().warn("AuthRequest rejected: invalid format (fd {})", session.getFileDescriptor());
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
			Logger::instance().warn("Bad DeleteRequest from fd {}", session.getFileDescriptor());
			break;
		}
		if (!Validator::validateUsername(data.username) || !Validator::validatePassword(data.password))
		{
			Logger::instance().warn("DeleteRequest rejected: invalid format (fd {})", session.getFileDescriptor());
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
			Logger::instance().warn("Bad FindUserRequest from fd {}", session.getFileDescriptor());
			break;
		}
		if (data.query.empty() || data.query.size() > Validator::MAX_USERNAME_LENGTH)
		{
			Logger::instance().warn("FindUserRequest rejected: bad query (fd {})", session.getFileDescriptor());
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
			Logger::instance().warn("Bad CreateChatRequest from fd {}", session.getFileDescriptor());
			break;
		}
		if (!Validator::validateUsername(data.peerUsername))
		{
			Logger::instance().warn("CreateChatRequest rejected: bad peer (fd {})", session.getFileDescriptor());
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
			Logger::instance().warn("Bad MessageSend from fd {}", session.getFileDescriptor());
			break;
		}
		if (!Validator::validateChatID(data.chatID) || !Validator::validateMessage(data.text))
		{
			Logger::instance().warn("MessageSend rejected: invalid format (fd {})", session.getFileDescriptor());
			break;
		}
		messageService_->handleMessageSend(header, data, session);
		break;
	}

	default:
		Logger::instance().warn("Unknown packet type 0x{:X} from fd {}",
			header.type, session.getFileDescriptor());
		break;
	}
}

void PacketDispatcher::handleConnectRequest(const PacketHeaderRaw& header, ClientSession& session)
{
	const uint32_t newSessionID = header.sessionID
		? header.sessionID
		: static_cast<uint32_t>(session.getFileDescriptor());

	ConnectResponseData resp;
	session.sendRaw(PacketBuilder::buildPacket(header.messageID, newSessionID, resp));
	Logger::instance().info("ConnectResponse sent to fd {} (sessionID={})",
		session.getFileDescriptor(), newSessionID);
}

void PacketDispatcher::handleDisconnectRequest(ClientSession& session)
{
	Logger::instance().info("Client {} requested disconnect", session.getFileDescriptor());
	session.closeSession();
}
