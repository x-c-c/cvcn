#include "Model.h"
#include "Packets.h"
#include <QDebug>

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

void Model::sendAuthRequest(const QString& username, const QString& password)
{

}
