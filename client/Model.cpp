#include "Model.h"
#include "PacketData.h"
#include <QDebug>
#include "PacketBuilder.h"
Model::Model(QObject* parent): QObject(parent), socket_(nullptr)
{
    socket_ = new QTcpSocket(this);
    connect( socket_, &QTcpSocket::connected, this, &Model::slotConnected);
    connect( socket_, &QTcpSocket::errorOccurred, this, &Model::slotSocketError);
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
    RegisterRequestData rrd;
    rrd.username = username.toStdString();
    rrd.password = password.toStdString();
    std::vector<uint8_t> packet = PacketBuilder::buildPacket(0,0, rrd);
    sendPacket(packet);
}






