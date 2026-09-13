#include "Model.h"
#include "Connection.h"
#include "PacketBuilder.h"
#include "PacketDeserializer.h"
#include <QDebug>

Model::Model(QObject* parent):
    QObject(parent), connection_(new Connection(this))
{
    connect(connection_, &Connection::connected,         this, &Model::onConnected);
    connect(connection_, &Connection::disconnected,      this, &Model::onDisconnected);
    connect(connection_, &Connection::errorOccurred,     this, &Model::onErrorOccurred);
    connect(connection_, &Connection::rawPacketReceived, this, &Model::onRawPacketReceived);
}

void Model::connectToServer(const QString& address, quint16 port)
{
    connection_->connectToServer(address, port);
}

void Model::onConnected()
{
    qDebug() << "connected";
    emit connected();
}

void Model::onDisconnected()
{
    qDebug() << "disconnected";
    emit disconnected();
}

void Model::onErrorOccurred(const QString& errorString)
{
    qDebug() << "Error:" << errorString;
    emit errorOccurred(errorString);
}

void Model::onRawPacketReceived(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
    processIncomingPacket(header, body);
}

void Model::processIncomingPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
    switch (static_cast<PacketType>(header.type))
    {
    case PacketType::RegisterResponse:
    {
        RegisterResponseData resp{};
        if (PacketDeserializer::deserializeData(body, resp))
            emit registrationFinished(resp.success == 1);
        break;
    }
    case PacketType::AuthResponse:
    {
        AuthResponseData resp{};
        if (PacketDeserializer::deserializeData(body, resp))
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
        if (PacketDeserializer::deserializeData(body, resp))
            emit deleteFinished(resp.success == 1);
        break;
    }
    case PacketType::FindUserResponse:
    {
        FindUserResponseData resp{};
        if (PacketDeserializer::deserializeData(body, resp))
            emit usersFound(resp.usernames);
        break;
    }
    case PacketType::CreateChatResponse:
    {
        CreateChatResponseData resp{};
        if (PacketDeserializer::deserializeData(body, resp))
            emit chatCreated(resp.success == 1, resp.chatID, QString::fromStdString(resp.peerUsername));
        break;
    }
    case PacketType::ChatListResponse:
    {
        ChatListResponseData resp{};
        if (PacketDeserializer::deserializeData(body, resp))
            emit chatListReceived(resp.chats);
        break;
    }
    case PacketType::MessageReceive:
    {
        MessageReceiveData recv{};
        if (PacketDeserializer::deserializeData(body, recv))
        {
            emit messageReceived(
                recv.senderID,
                QString::fromStdString(recv.senderUsername),
                recv.chatID,
                QString::fromStdString(recv.text));
        }
        break;
    }
    default:
        qDebug() << "Unknown packet type:" << static_cast<int>(header.type);
        break;
    }
}

void Model::sendPacket(const std::vector<uint8_t>& packet)
{
    connection_->send(packet);
}

void Model::sendRegRequest(const QString& username, const QString& password)
{
    RegisterRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void Model::sendAuthRequest(const QString& username, const QString& password)
{
    AuthRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void Model::sendDeleteRequest(const QString& username, const QString& password)
{
    DeleteRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void Model::sendMessage(uint32_t chatID, const QString& text)
{
    MessageSendData payload;
    payload.senderID = sessionID_;
    payload.chatID   = chatID;
    payload.text     = text.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void Model::sendFindUserRequest(const QString& query)
{
    FindUserRequestData payload;
    payload.query = query.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void Model::sendCreateChatRequest(const QString& peerUsername)
{
    CreateChatRequestData payload;
    payload.peerUsername = peerUsername.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void Model::sendChatListRequest()
{
    ChatListRequestData payload;
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void Model::increaseMessageID()
{
    ++messageID_;
}
