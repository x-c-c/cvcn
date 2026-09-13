#include "ClientSession.h"
#include "Epoller.h"
#include "PacketBuilder.h"
#include "PacketDeserializer.h"
#include "Logger.h"
#include "Validator.h"
#include "UserRepository.h"
#include "ChatRepository.h"
#include "MessageRepository.h"
#include "SessionRegistry.h"
#include <cstring>
#include <cerrno>
#include <unistd.h>

ClientSession::ClientSession(int fileDescriptor,
							 Epoller* epoller,
							 UserRepository* userRepo,
							 ChatRepository* chatRepo,
							 MessageRepository* msgRepo,
							 SessionRegistry* sessionRegistry):
	fileDescriptor_(fileDescriptor),
	epoller_(epoller),
	userRepo_(userRepo),
	chatRepo_(chatRepo),
	msgRepo_(msgRepo),
	sessionRegistry_(sessionRegistry),
	sender_(epoller, fileDescriptor){}

ClientSession::~ClientSession()
{
	if (!closed_)
		closeSession();
}

void ClientSession::handleRead()
{
	uint8_t tempBuffer[TEMP_BUFFER_SIZE];
	ssize_t bytesRead = recv(fileDescriptor_, tempBuffer, sizeof(tempBuffer), 0);
	if (bytesRead > 0)
	{
		assembler_.appendData(tempBuffer, bytesRead);
		PacketHeaderRaw header;
		std::vector<uint8_t> body;
		while (assembler_.extractPacket(header, body))
		{
			processPacket(header, body);
			if (closed_)
				return;
		}
	}
	else if (bytesRead == 0)
	{
		Logger::instance().info("Client {} closed connection", fileDescriptor_);
		closeSession();
	}
	else if (errno != EAGAIN && errno != EWOULDBLOCK)
	{
		Logger::instance().error("recv error on fd {}: {}", fileDescriptor_, strerror(errno));
		closeSession();
	}
}

void ClientSession::handleWrite()
{
	sender_.handleWrite();
}

void ClientSession::processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
	if (!validateIncomingPacket(header, body))
	{
		Logger::instance().warn("Packet 0x{:X} rejected from fd {}: validation failed", header.type, fileDescriptor_);
		return;
	}

	switch (static_cast<PacketType>(header.type))
	{
	case PacketType::ConnectRequest:
		handleConnectRequestData(header.messageID, header.sessionID);
		break;
	case PacketType::RegisterRequest:
	{
		RegisterRequestData data;
		if (PacketDeserializer::deserializeData(body, data))
			handleRegisterRequestData(header.messageID, header.sessionID, data);
		else
			Logger::instance().warn("Failed to parse RegisterRequest from fd {}", fileDescriptor_);
		break;
	}
	case PacketType::AuthRequest:
	{
		AuthRequestData data;
		if (PacketDeserializer::deserializeData(body, data))
			handleAuthRequestData(header.messageID, header.sessionID, data);
		else
			Logger::instance().warn("Failed to parse AuthRequest from fd {}", fileDescriptor_);
		break;
	}
	case PacketType::MessageSend:
	{
		MessageSendData data;
		if (PacketDeserializer::deserializeData(body, data))
			handleMessageSendData(header.messageID, header.sessionID, data);
		else
			Logger::instance().warn("Failed to parse MessageSend from fd {}", fileDescriptor_);
		break;
	}
	case PacketType::DisconnectRequest:
		handleDisconnectRequestData();
		break;
	case PacketType::DeleteRequest:
	{
		DeleteRequestData data;
		if (PacketDeserializer::deserializeData(body, data))
			handleDeleteRequestData(header.messageID, header.sessionID, data);
		else
			Logger::instance().warn("Failed to parse DeleteRequest from fd {}", fileDescriptor_);
		break;
	}
	case PacketType::FindUserRequest:
	{
		FindUserRequestData data;
		if (PacketDeserializer::deserializeData(body, data))
			handleFindUserRequestData(header.messageID, header.sessionID, data);
		break;
	}
	case PacketType::CreateChatRequest:
	{
		CreateChatRequestData data;
		if (PacketDeserializer::deserializeData(body, data))
			handleCreateChatRequestData(header.messageID, header.sessionID, data);
		break;
	}
	case PacketType::ChatListRequest:
		handleChatListRequestData(header.messageID, header.sessionID);
		break;
	default:
		Logger::instance().warn("Unknown packet type 0x{:X} from client {}", header.type, fileDescriptor_);
		break;
	}
}

void ClientSession::handleConnectRequestData(uint32_t messageID, uint32_t sessionID)
{
	uint32_t newSessionID = sessionID ? sessionID : static_cast<uint32_t>(fileDescriptor_);
	ConnectResponseData resp;
	auto response = PacketBuilder::buildPacket(messageID, newSessionID, resp);
	sender_.sendResponse(response);
	Logger::instance().info("ConnectResponse sent to fd {} (sessionID={})", fileDescriptor_, newSessionID);
}

void ClientSession::handleRegisterRequestData(uint32_t messageID, uint32_t sessionID, const RegisterRequestData& data)
{
	RegisterResponseData resp;
	if (userRepo_->isUserExist(data.username))
	{
		resp.success = 0;
		Logger::instance().warn("Register failed for '{}' (fd {}): user already exists",
			data.username, fileDescriptor_);
	}
	else
	{
		const std::string hash = "hash_" + data.password;
		resp.success = userRepo_->addUser(data.username, hash) ? 1 : 0;
		if (resp.success)
			Logger::instance().info("Register OK for '{}' (fd {})",
				data.username, fileDescriptor_);
		else
			Logger::instance().error("Register failed for '{}' (fd {}): DB error",
				data.username, fileDescriptor_);
	}
	auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
	sender_.sendResponse(response);
}

void ClientSession::handleAuthRequestData(uint32_t messageID, uint32_t sessionID, const AuthRequestData& data)
{
	AuthResponseData resp;
	const std::string storedHash = userRepo_->getUserPasswordHash(data.username);
	resp.success = (!storedHash.empty() && storedHash == "hash_" + data.password) ? 1 : 0;

	if (resp.success)
	{
		userID_ = userRepo_->getUserID(data.username);
		username_ = data.username;

		if (sessionRegistry_)
			sessionRegistry_->registerUser(userID_, this);

		Logger::instance().info("Auth OK for '{}' (fd {}, uid {})",
			data.username, fileDescriptor_, userID_);
	}
	else
	{
		Logger::instance().warn("Auth failed for '{}' (fd {})",
			data.username, fileDescriptor_);
	}

	auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
	sender_.sendResponse(response);
}

void ClientSession::handleMessageSendData(uint32_t messageID, uint32_t sessionID, const MessageSendData& data)
{
	// Шаг 5: здесь будет сохранение в MessageRepository и рассылка через SessionRegistry.
	Logger::instance().info("Message from sender={} to chat={} (fd {}): '{}'",
		data.senderID, data.chatID, fileDescriptor_, data.text);
	(void)messageID;
	(void)sessionID;
}

void ClientSession::handleDisconnectRequestData()
{
	Logger::instance().info("Client {} requested disconnect", fileDescriptor_);
	closeSession();
}

void ClientSession::handleDeleteRequestData(uint32_t messageID, uint32_t sessionID, const DeleteRequestData& data)
{
	DeleteResponseData resp;
	const std::string hash = "hash_" + data.password;
	resp.success = userRepo_->deleteUser(data.username, hash) ? 1 : 0;

	if (resp.success)
		Logger::instance().info("Delete OK for '{}' (fd {})",
			data.username, fileDescriptor_);
	else
		Logger::instance().warn("Delete failed for '{}' (fd {}): user not found or wrong password",
			data.username, fileDescriptor_);

	auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
	sender_.sendResponse(response);
}

void ClientSession::handleFindUserRequestData(uint32_t messageID, uint32_t sessionID, const FindUserRequestData& data)
{
	if (userID_ == -1)
	{
		Logger::instance().warn("FindUserRequest before auth (fd {})", fileDescriptor_);
		FindUserResponseData resp;
		auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
		sender_.sendResponse(response);
		return;
	}

	FindUserResponseData resp;
	resp.usernames = chatRepo_->findUsers(data.query, userID_);
	auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
	sender_.sendResponse(response);
	Logger::instance().info("FindUser '{}' from fd {} → {} results",
		data.query, fileDescriptor_, resp.usernames.size());
}

void ClientSession::handleCreateChatRequestData(uint32_t messageID, uint32_t sessionID, const CreateChatRequestData& data)
{
	CreateChatResponseData resp{};
	if (userID_ == -1)
	{
		resp.success = 0;
	}
	else
	{
		const int peerID = userRepo_->getUserID(data.peerUsername);
		if (peerID == -1 || peerID == userID_)
		{
			resp.success = 0;
			Logger::instance().warn("CreateChat failed for '{}': peer not found (fd {})",
				data.peerUsername, fileDescriptor_);
		}
		else
		{
			const int chatID = chatRepo_->findOrCreateDirectChat(userID_, peerID);
			if (chatID > 0)
			{
				resp.success = 1;
				resp.chatID = static_cast<uint32_t>(chatID);
				resp.peerUsername = data.peerUsername;
				Logger::instance().info("Chat {} created/found between '{}' and '{}'",
					chatID, username_, data.peerUsername);
			}
			else
			{
				resp.success = 0;
			}
		}
	}
	auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
	sender_.sendResponse(response);
}

void ClientSession::handleChatListRequestData(uint32_t messageID, uint32_t sessionID)
{
	ChatListResponseData resp;
	if (userID_ != -1)
		resp.chats = chatRepo_->getUserChats(userID_);
	auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
	sender_.sendResponse(response);
}

void ClientSession::closeSession()
{
	if (closed_)
		return;

	if (sessionRegistry_ && userID_ != -1)
		sessionRegistry_->unregisterUser(userID_);

	epoller_->removeFdFromEpoll(fileDescriptor_);
	close(fileDescriptor_);
	closed_ = true;
	Logger::instance().info("Session closed for fd {}", fileDescriptor_);
}

bool ClientSession::validateIncomingPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
	switch (static_cast<PacketType>(header.type))
	{
	case PacketType::RegisterRequest:
	{
		RegisterRequestData data;
		if (!PacketDeserializer::deserializeData(body, data))
			return false;
		return Validator::validateUsername(data.username) && Validator::validatePassword(data.password);
	}
	case PacketType::AuthRequest:
	{
		AuthRequestData data;
		if (!PacketDeserializer::deserializeData(body, data))
			return false;
		return Validator::validateUsername(data.username) && Validator::validatePassword(data.password);
	}
	case PacketType::DeleteRequest:
	{
		DeleteRequestData data;
		if (!PacketDeserializer::deserializeData(body, data))
			return false;
		return Validator::validateUsername(data.username) && Validator::validatePassword(data.password);
	}
	case PacketType::MessageSend:
	{
		MessageSendData data;
		if (!PacketDeserializer::deserializeData(body, data))
			return false;
		return Validator::validateChatID(data.chatID)
			&& Validator::validateMessage(data.text);
	}
	case PacketType::FindUserRequest:
	{
		FindUserRequestData data;
		if (!PacketDeserializer::deserializeData(body, data))
			return false;
		return !data.query.empty() && data.query.size() <= Validator::MAX_USERNAME_LENGTH;
	}
	case PacketType::CreateChatRequest:
	{
		CreateChatRequestData data;
		if (!PacketDeserializer::deserializeData(body, data))
			return false;
		return Validator::validateUsername(data.peerUsername);
	}
	default:
		return true;
	}
}
