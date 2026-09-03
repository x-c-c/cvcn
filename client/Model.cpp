#include "Model.h"
#include "PacketData.h"
#include <QDebug>
#include "PacketBuilder.h"
#include "PacketDeserializer.h"
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
}
void Model::slotReadyRead()
{
    QByteArray chunk = socket_->readAll();
    receiveBuffer_.append(chunk);
    while (receiveBuffer_.size() >sizeof(PacketHeaderRaw))
    {
        PacketHeaderRaw header;
        PacketParser::deserializeHeader(receiveBuffer_, header);
        if  (receiveBuffer_.size() >=  header.messageLen)
        {

        }
    }
}



void Model::sendPacket(const std::vector<uint8_t>& packet)
{
    if (socket_->state() != QAbstractSocket::ConnectedState)
    {
        return;
    }
    QByteArray data = QByteArray::fromRawData(
        reinterpret_cast<const char*>(packet.data()),
        static_cast<int>(packet.size())
        );

    qint64 bytesWritten  =  socket_->write(data);
    if (bytesWritten == -1)
    {
        emit errorOccurred(socket_->errorString());
    }
}


void Model::sendRegRequest(const QString& username, const QString& password)
{
    RegisterRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    std::vector<uint8_t> packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}

void Model::increaseMessageID()
{
    ++messageID_;
}









