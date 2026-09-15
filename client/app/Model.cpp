#include <QDebug>
#include "./Model.h"
#include "../protocol/PacketData.h"
#include "../protocol/PacketBuilder.h"
#include "../protocol/PacketParser.h"
Model::Model(QObject* parent): QObject(parent), socket_(nullptr)
{
    socket_ = new QTcpSocket(this);
    connect( socket_, &QTcpSocket::connected, this, &Model::slotConnected);
    connect( socket_, &QTcpSocket::errorOccurred, this, &Model::slotSocketError);
    connect( socket_, &QTcpSocket::readyRead, this, &Model::slotReadyRead);
}
void Model::connectToServer(const QString& address, const quint16 port)
{
    socket_->connectToHost(address, port);
}

void Model::slotConnected()
{
    qDebug() << "connected";
}
void Model::slotSocketError(QAbstractSocket::SocketError error)
{
    qDebug() << "Socket error:" << socket_->errorString() << "(code " << error << " )";
    emit errorOccurred(socket_->errorString());
}
void Model::slotReadyRead()
{
    const QByteArray chunk = socket_->readAll();
    receiveBuffer_.insert(receiveBuffer_.end(), chunk.begin(), chunk.end());

    while (receiveBuffer_.size() >= sizeof(PacketHeaderRaw))
    {
        PacketHeaderRaw header;
        if (!PacketParser::deserializeHeader(receiveBuffer_, header))
            return;

        const std::size_t totalSize = sizeof(PacketHeaderRaw) + header.messageLen;
        if (receiveBuffer_.size() < totalSize)
            return;   // ждём остаток

        std::vector<uint8_t> body(receiveBuffer_.begin() + sizeof(PacketHeaderRaw), receiveBuffer_.begin() + totalSize);

        receiveBuffer_.erase(receiveBuffer_.begin(), receiveBuffer_.begin() + totalSize);

        qDebug() << "Packet type:" << static_cast<int>(header.type)
                 << "messageID:"   << header.messageID
                 << "sessionID:"   << header.sessionID
                 << "len:"         << header.messageLen;
    }
}

void Model::sendPacket(const std::vector<uint8_t>& packet)
{
    if (socket_->state() != QAbstractSocket::ConnectedState)
    {
        emit errorOccurred(QStringLiteral("Not connected to server"));
        return;
    }

    const QByteArray data(reinterpret_cast<const char*>(packet.data()), static_cast<int>(packet.size()));
    const qint64 bytesWritten = socket_->write(data);
    if (bytesWritten == -1)
        emit errorOccurred(socket_->errorString());
}

void Model::sendRegRequest(const QString& username, const QString& password)
{
    RegisterRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    const auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}

void Model::sendAuthRequest(const QString& username, const QString& password)
{
    AuthRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    const auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}

/*
void Model::sendDelRequest(const QString& username, const QString& password)
{
    DeleteRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    const auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}
*/
void Model::increaseMessageID()
{
    ++messageID_;
}
