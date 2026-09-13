#include "Connection.h"
#include "PacketParser.h"

Connection::Connection(QObject* parent):
    QObject(parent), socket_(new QTcpSocket(this))
{
    connect(socket_, &QTcpSocket::connected,     this, &Connection::slotConnected);
    connect(socket_, &QTcpSocket::disconnected,  this, &Connection::slotDisconnected);
    connect(socket_, &QTcpSocket::errorOccurred, this, &Connection::slotSocketError);
    connect(socket_, &QTcpSocket::readyRead,     this, &Connection::slotReadyRead);
}

void Connection::connectToServer(const QString& address, quint16 port)
{
    socket_->connectToHost(address, port);
}

void Connection::send(const std::vector<uint8_t>& packet)
{
    if (socket_->state() != QAbstractSocket::ConnectedState)
    {
        emit signalErrorOccurred(QStringLiteral("Not connected to server"));
        return;
    }

    QByteArray data = QByteArray::fromRawData(
        reinterpret_cast<const char*>(packet.data()),
        static_cast<int>(packet.size()));

    const qint64 bytesWritten = socket_->write(data);
    if (bytesWritten == -1)
        emit signalErrorOccurred(socket_->errorString());
}

bool Connection::isConnected() const
{
    return socket_->state() == QAbstractSocket::ConnectedState;
}

void Connection::slotConnected()
{
    emit signalConnected();
}

void Connection::slotDisconnected()
{
    emit signalDisconnected();
}

void Connection::slotSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    emit signalErrorOccurred(socket_->errorString());
}

void Connection::slotReadyRead()
{
    const QByteArray chunk = socket_->readAll();
    receiveBuffer_.insert(receiveBuffer_.end(), chunk.begin(), chunk.end());

    while (receiveBuffer_.size() >= sizeof(PacketHeaderRaw))
    {
        PacketHeaderRaw header;
        if (!PacketParser::parseHeader(receiveBuffer_, header))
            break;

        const size_t totalSize = sizeof(PacketHeaderRaw) + header.messageLen;
        if (receiveBuffer_.size() < totalSize)
            break;

        std::vector<uint8_t> body(
            receiveBuffer_.begin() + sizeof(PacketHeaderRaw),
            receiveBuffer_.begin() + totalSize);

        receiveBuffer_.erase(receiveBuffer_.begin(), receiveBuffer_.begin() + totalSize);

        emit signalRawPacketReceived(header, body);
    }
}
