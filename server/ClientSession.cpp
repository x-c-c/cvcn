#include "ClientSession.h"
#include "Epoller.h"
#include "PacketBuilder.h"
#include "PacketDeserializer.h"
#include "Logger.h"
#include <cstring>
#include <cerrno>
#include <unistd.h>

ClientSession::ClientSession(int fileDescriptor, Epoller* epoller, Database* db):
	fileDescriptor_(fileDescriptor), epoller_(epoller), db_(db), sender_(epoller, fileDescriptor){}

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
		break;
	}
	case PacketType::AuthRequest:
	{
		AuthRequestData data;
		if (PacketDeserializer::deserializeData(body, data))
			handleAuthRequestData(header.messageID, header.sessionID, data);
		break;
	}
	case PacketType::MessageSend:
	{
		MessageSendData data;
		if (PacketDeserializer::deserializeData(body, data))
			handleMessageSendData(header.messageID, header.sessionID, data);
		break;
	}
	case PacketType::DisconnectRequest:
		handleDisconnectRequestData();
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
}

void ClientSession::handleRegisterRequestData(uint32_t messageID, uint32_t sessionID, const RegisterRequestData& data)
{
	RegisterResponseData resp;
	if (db_->isUserExist(data.username))
	{
		resp.success = 0;
	}
	else
	{
		std::string hash = "hash_" + data.password;
		resp.success = db_->addUser(data.username, hash) ? 1 : 0;
	}
	auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
	sender_.sendResponse(response);
}

void ClientSession::handleAuthRequestData(uint32_t messageID, uint32_t sessionID,
	const AuthRequestData& data)
{
	AuthResponseData resp;
	std::string storedHash = db_->getUserPasswordHash(data.username);
	resp.success = (!storedHash.empty() && storedHash == "hash_" + data.password) ? 1 : 0;

	auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
	sender_.sendResponse(response);
}

void ClientSession::handleMessageSendData(uint32_t messageID, uint32_t sessionID, const MessageSendData& data)
{
	Logger::instance().info("Message from {} to chat {}: {}", data.senderID, data.chatID, data.text);
	(void)messageID;
	(void)sessionID;
}

void ClientSession::handleDisconnectRequestData()
{
	Logger::instance().info("Client {} requested disconnect", fileDescriptor_);
	closeSession();
}

void ClientSession::closeSession()
{
	if (closed_)
		return;
	epoller_->removeFdFromEpoll(fileDescriptor_);
	close(fileDescriptor_);
	closed_ = true;
	Logger::instance().info("Session closed for fd {}", fileDescriptor_);
}
