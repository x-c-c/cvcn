#include "ProtocolClient.h"
#include "Connection.h"
#include "PacketBuilder.h"
#include "PacketParser.h"
#include "Logger.h"
#include <QDebug>

ProtocolClient::ProtocolClient(QObject* parent):
    QObject(parent), connection_(new Connection(this))
{
    connect(connection_, &Connection::connected,         this, &ProtocolClient::onConnected);
    connect(connection_, &Connection::disconnected,      this, &ProtocolClient::onDisconnected);
    connect(connection_, &Connection::errorOccurred,     this, &ProtocolClient::onErrorOccurred);
    connect(connection_, &Connection::rawPacketReceived, this, &ProtocolClient::onRawPacketReceived);
}

void ProtocolClient::connectToServer(const QString& address, quint16 port)
{
    connection_->connectToServer(address, port);
}

void ProtocolClient::onConnected()
{
    LOG_INFO("connected");
    emit connected();
}

void ProtocolClient::onDisconnected()
{
    LOG_INFO("disconnected");
    emit disconnected();
}

void ProtocolClient::onErrorOccurred(const QString& errorString)
{
    LOG_ERROR("{}", errorString.toStdString());
    emit errorOccurred(errorString);
}

void ProtocolClient::onRawPacketReceived(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
    processIncomingPacket(header, body);
}

void ProtocolClient::processIncomingPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
    switch (static_cast<PacketType>(header.type))
    {
    case PacketType::RegisterResponse:
    {
        RegisterResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit registrationFinished(resp.success == 1);
        break;
    }
    case PacketType::AuthResponse:
    {
        AuthResponseData resp{};
        if (PacketParser::parseData(body, resp))
        {
            const bool success = (resp.success == 1);
            if (success)
                sessionID_ = header.sessionID;
            emit authFinished(success, sessionID_);
        }
        break;
    }
    case PacketType::DeleteResponse:
    {
        DeleteResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit deleteFinished(resp.success == 1);
        break;
    }
    case PacketType::FindUserResponse:
    {
        FindUserResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit usersFound(resp.usernames);
        break;
    }
    case PacketType::CreateChatResponse:
    {
        CreateChatResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit chatCreated(resp.success == 1, resp.chatID, QString::fromStdString(resp.peerUsername));
        break;
    }
    case PacketType::ChatListResponse:
    {
        ChatListResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit chatListReceived(resp.chats);
        break;
    }
    case PacketType::MessageReceive:
    {
        MessageReceiveData recv{};
        if (PacketParser::parseData(body, recv))
        {
            emit messageReceived(
                QString::fromStdString(recv.senderUsername),
                recv.chatID,
                QString::fromStdString(recv.text));
        }
        break;
    }
    default:
        LOG_WARN("Unknown packet type: {}", static_cast<int>(header.type));
        break;
    }
}

void ProtocolClient::sendPacket(const std::vector<uint8_t>& packet)
{
    connection_->send(packet);
}

void ProtocolClient::sendRegRequest(const QString& username, const QString& password)
{
    RegisterRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendAuthRequest(const QString& username, const QString& password)
{
    AuthRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendDeleteRequest(const QString& username, const QString& password)
{
    DeleteRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendMessage(uint32_t chatID, const QString& text)
{
    MessageSendData payload;
    payload.chatID   = chatID;
    payload.text     = text.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendFindUserRequest(const QString& query)
{
    FindUserRequestData payload;
    payload.query = query.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendCreateChatRequest(const QString& peerUsername)
{
    CreateChatRequestData payload;
    payload.peerUsername = peerUsername.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendChatListRequest()
{
    ChatListRequestData payload;
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::increaseMessageID()
{
    ++messageID_;
}
