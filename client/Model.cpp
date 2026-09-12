#include "Model.h"
#include "PacketData.h"
#include "PacketBuilder.h"
#include "PacketDeserializer.h"
#include <QDebug>
#include <cstring>

Model::Model(QObject* parent): QObject(parent), socket_(nullptr)
{
    socket_ = new QTcpSocket(this);
    connect(socket_, &QTcpSocket::connected,    this, &Model::slotConnected);
    connect(socket_, &QTcpSocket::errorOccurred, this, &Model::slotSocketError);
    connect(socket_, &QTcpSocket::readyRead,    this, &Model::slotReadyRead);
}

void Model::connectToServer(const QString& address, quint16 port)
{
    socket_->connectToHost(address, port);
}

void Model::slotConnected()
{
    qDebug() << "connected";
    emit connected();
}

void Model::slotSocketError(QAbstractSocket::SocketError error)
{
    qDebug() << "Socket error:" << socket_->errorString() << "(code" << error << ")";
    emit errorOccurred(socket_->errorString());
}

void Model::slotReadyRead()
{
    QByteArray chunk = socket_->readAll();
    receiveBuffer_.insert(receiveBuffer_.end(), chunk.begin(), chunk.end());

    while (receiveBuffer_.size() >= sizeof(PacketHeaderRaw))
    {
        PacketHeaderRaw header;
        if (!PacketDeserializer::deserializeHeader(receiveBuffer_, header))
            break;

        size_t totalSize = sizeof(PacketHeaderRaw) + header.messageLen;
        if (receiveBuffer_.size() < totalSize)
            break;

        std::vector<uint8_t> body(
            receiveBuffer_.begin() + sizeof(PacketHeaderRaw),
            receiveBuffer_.begin() + totalSize);

        receiveBuffer_.erase(receiveBuffer_.begin(), receiveBuffer_.begin() + totalSize);

        processIncomingPacket(header, body);
    }
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
            bool success = (resp.success == 1);
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
    default:
        qDebug() << "Unknown packet type:" << static_cast<int>(header.type);
        break;
    }
}

void Model::sendPacket(const std::vector<uint8_t>& packet)
{
    if (socket_->state() != QAbstractSocket::ConnectedState)
    {
        emit errorOccurred("Not connected to server");
        return;
    }
    QByteArray data = QByteArray::fromRawData(
        reinterpret_cast<const char*>(packet.data()),
        static_cast<int>(packet.size()));
    qint64 bytesWritten = socket_->write(data);
    if (bytesWritten == -1)
        emit errorOccurred(socket_->errorString());
}

void Model::sendRegRequest(const QString& username, const QString& password)
{
    RegisterRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}

void Model::sendAuthRequest(const QString& username, const QString& password)
{
    AuthRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}
void Model::sendDeleteRequest(const QString& username, const QString& password)
{
    DeleteRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}
void Model::increaseMessageID()
{
    ++messageID_;
}

void Model::sendMessage(uint32_t chatID, const QString& text)
{
    MessageSendData payload;
    payload.senderID = sessionID_;   // пока 0, потом заменим
    payload.chatID   = chatID;
    payload.text     = text.toStdString();
    auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
    emit messageSent(chatID, text);
}

void Model::sendFindUserRequest(const QString& query)
{
    FindUserRequestData payload;
    payload.query = query.toStdString();
    auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}

void Model::sendCreateChatRequest(const QString& peerUsername)
{
    CreateChatRequestData payload;
    payload.peerUsername = peerUsername.toStdString();
    auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}

void Model::sendChatListRequest()
{
    ChatListRequestData payload;
    auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}
