#include "ClientSession.h"
#include "Epoller.h"
#include "Serializer.h"
#include "Logger.h"
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>

ClientSession::ClientSession(int socketDescriptor, Epoller* epoller, Database* db):
	socketDescriptor_(socketDescriptor), epoller_(epoller), db_(db), sender_(epoller, socketDescriptor){}

ClientSession::~ClientSession()
{
    if (!closed_)
        closeSession();
}

void ClientSession::handleRead()
{
    uint8_t tempBuffer[4096];
    ssize_t bytesRead = recv(socketDescriptor_, tempBuffer, sizeof(tempBuffer), 0);
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
        Logger::instance().info("Client {} closed connection", socketDescriptor_);
        closeSession();
    }
    else if (errno != EAGAIN && errno != EWOULDBLOCK)
    {
        Logger::instance().error("recv error on fd {}: {}", socketDescriptor_, strerror(errno));
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
        {
            ConnectRequestPacket packet;
            Serializer::parseConnectRequestPacket(body, packet);
            handleConnectRequestPacket(header.messageID, header.sessionID);
        }
        break;
    case PacketType::RegisterRequest:
        {
            RegisterRequestPacket packet;
            if (Serializer::parseRegisterRequestPacket(body, packet))
                handleRegisterRequestPacket(header.messageID, header.sessionID, packet);
        }
        break;
    case PacketType::AuthRequest:
        {
            AuthRequestPacket packet;
            if (Serializer::parseAuthRequestPacket(body, packet))
                handleAuthRequestPacket(header.messageID, header.sessionID, packet);
        }
        break;
    case PacketType::MessageSend:
        {
            MessageSendPacket packet;
            if (Serializer::parseMessageSendPacket(body, packet))
                handleMessageSendPacket(header.messageID, header.sessionID, packet);
        }
        break;
    case PacketType::DisconnectRequest:
        handleDisconnectRequestPacket();
        break;
    default:
        Logger::instance().warn("Unknown packet type 0x{:X} from client {}", header.type, socketDescriptor_);
        break;
    }
}

void ClientSession::handleConnectRequestPacket(uint32_t messageID, uint32_t sessionID)
{
    uint32_t newSessionID = sessionID ? sessionID : static_cast<uint32_t>(socketDescriptor_);
    auto response = Serializer::buildConnectResponsePacket(messageID, newSessionID);
    sender_.sendResponse(response);
}

void ClientSession::handleRegisterRequestPacket(uint32_t messageID, uint32_t sessionID,
    const RegisterRequestPacket& packet)
{
    RegisterResponsePacket resp;
    if (db_->isUserExist(packet.username))
    {
        resp.success = 0;
    }
    else
    {
        std::string hash = "hash_" + packet.password;
        if (db_->addUser(packet.username, hash))
            resp.success = 1;
        else
            resp.success = 0;
    }
    auto response = Serializer::buildRegisterResponsePacket(messageID, sessionID, resp);
    sender_.sendResponse(response);
}

void ClientSession::handleAuthRequestPacket(uint32_t messageID, uint32_t sessionID,
    const AuthRequestPacket& packet)
{
    AuthResponsePacket resp;
    std::string storedHash = db_->getUserPasswordHash(packet.username);
    if (!storedHash.empty() && storedHash == "hash_" + packet.password)
        resp.success = 1;
    else
        resp.success = 0;

    auto response = Serializer::buildAuthResponsePacket(messageID, sessionID, resp);
    sender_.sendResponse(response);
}

void ClientSession::handleMessageSendPacket(uint32_t messageID, uint32_t sessionID,
    const MessageSendPacket& packet)
{
    Logger::instance().info("Message from {} to chat {}: {}", packet.senderID, packet.chatID, packet.text);
}

void ClientSession::handleDisconnectRequestPacket()
{
    Logger::instance().info("Client {} requested disconnect", socketDescriptor_);
    closeSession();
}

void ClientSession::closeSession()
{
    if (closed_)
        return;
    closed_ = true;
    close(socketDescriptor_);
    Logger::instance().info("Session closed for fd {}", socketDescriptor_);
}
