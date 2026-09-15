#include "./ClientSession.h"
#include "./EventPoller.h"
#include "../protocol/PacketParser.h"
#include "../protocol/PacketBuilder.h"
#include "../storage/Database.h"
#include "../utils/Logger.h"
#include <cstring>
#include <cerrno>
#include <unistd.h>

ClientSession::ClientSession(int socketFD, EventPoller* eventPoller, Database* db):
	socketFD_(socketFD), eventPoller_(eventPoller), db_(db), sender_(eventPoller, socketFD){}

ClientSession::~ClientSession()
{
    if (!closed_)
        closeSession();
}

void ClientSession::handleRead()
{
    uint8_t tempBuffer[TEMP_BUFFER_SIZE];
    ssize_t bytesRead = recv(socketFD_, tempBuffer, sizeof(tempBuffer), 0);
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
        Logger::instance().info("Client {} closed connection", socketFD_);
        closeSession();
    }
    else if (errno != EAGAIN && errno != EWOULDBLOCK)
    {
        Logger::instance().error("recv error on fd {}: {}", socketFD_, strerror(errno));
        closeSession();
    }
}

void ClientSession::handleWrite()
{
    sender_.handleWrite();
}

void ClientSession::processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
	const uint32_t messageID = header.messageID;
    const uint32_t sessionID = header.sessionID;
    switch (static_cast<PacketType>(header.type))
    {
		case PacketType::ConnectRequest:
		{
			ConnectRequestData data;
			if (PacketParser::deserializeData(body, data))
				handlePacket(header.messageID, header.sessionID, data);
			break;
		}
		case PacketType::RegisterRequest:
		{
			RegisterRequestData data;
			if (PacketParser::deserializeData(body, data))
				handlePacket(header.messageID, header.sessionID, data);
			break;
		}
		case PacketType::AuthRequest:
		{
			AuthRequestData data;
			if (PacketParser::deserializeData(body, data))
				handlePacket(header.messageID, header.sessionID, data);
			break;
		}
		case PacketType::MessageSend:
		{
			MessageSendData data;
			if (PacketParser::deserializeData(body, data))
				handlePacket(header.messageID, header.sessionID, data);
			break;
		}
		case PacketType::DisconnectRequest:
			handlePacket();
			break;
		default:
			Logger::instance().warn("Unknown packet type 0x{:X} from client {}", static_cast<unsigned>(header.type), socketFD_);
			break;
    }
}

void ClientSession::handlePacket(uint32_t messageID, uint32_t /*sessionID*/, const ConnectRequestData& /*data*/)
{
    sessionID_ = static_cast<uint32_t>(socketFD_);
    const ConnectResponseData responseData;
    const auto response = PacketBuilder::buildPacket(messageID, sessionID_, responseData);
    sender_.sendResponse(response);
}

void ClientSession::handlePacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestData& data)
{
    RegisterResponseData resp{};
    if (db_->isUserExist(data.username))
    {
        resp.success = 0;
    }
    else
    {
        const std::string hash = "hash_" + data.password;
        resp.success = db_->addUser(data.username, hash) ? 1 : 0;
    }

    const auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
    sender_.sendResponse(response);
}

void ClientSession::handlePacket(uint32_t messageID, uint32_t sessionID, const AuthRequestData& data)
{
    AuthResponseData resp{};
    const std::string storedHash = db_->getUserPasswordHash(data.username);
    resp.success = (!storedHash.empty() && storedHash == "hash_" + data.password) ? 1 : 0;
    const auto response = PacketBuilder::buildPacket(messageID, sessionID, resp);
    sender_.sendResponse(response);
}

void ClientSession::handlePacket(uint32_t /*messageID*/, uint32_t /*sessionID*/, const MessageSendData& data)
{
    Logger::instance().info("Message from {} to chat {}: {}", data.senderID, data.chatID, data.text);
    // TODO: разослать MessageReceive участникам чата
}

void ClientSession::handlePacket()
{
    Logger::instance().info("Client {} requested disconnect", socketFD_);
    closeSession();
}

void ClientSession::closeSession()
{
    if (closed_)
        return;
    closed_ = true;
    close(socketFD_);
    Logger::instance().info("Session closed for fd {}", socketFD_);
}
