#include "Connection.h"
#include "PacketParser.h"
#include "AppConfig.h"
#include <new>

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
        emit signalErrorOccurred(ErrorKind::Transport,
                                 QStringLiteral("Not connected to server"));
        return;
    }

    if (packet.empty())
        return;

    try
    {
        // fromRawData не копирует, но write() — копирует во внутренний буфер.
        // QByteArray тут живёт до конца функции, что безопасно.
        QByteArray data = QByteArray::fromRawData(
            reinterpret_cast<const char*>(packet.data()),
            static_cast<int>(packet.size()));

        const qint64 bytesWritten = socket_->write(data);
        if (bytesWritten == -1)
            emit signalErrorOccurred(ErrorKind::Transport, socket_->errorString());
        else
            socket_->flush();
    }
    catch (const std::bad_alloc&)
    {
        emit signalErrorOccurred(ErrorKind::Transport,
                                 QStringLiteral("Out of memory in send()"));
    }
    catch (const std::exception& e)
    {
        emit signalErrorOccurred(ErrorKind::Transport,
                                 QString::fromUtf8(e.what()));
    }
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
    emit signalErrorOccurred(ErrorKind::Transport, socket_->errorString());
}

void Connection::slotReadyRead()
{
    try
    {
        const QByteArray chunk = socket_->readAll();
        receiveBuffer_.insert(receiveBuffer_.end(), chunk.begin(), chunk.end());

        while (receiveBuffer_.size() >= sizeof(PacketHeaderRaw))
        {
            PacketHeaderRaw header;
            if (!PacketParser::parseHeader(receiveBuffer_, header))
                break;

            // Защита от мусора в поле messageLen. Легитимный пакет
            // не может быть больше, чем MAX_MESSAGE_LENGTH + служебные поля.
            // Верхняя граница нужна, чтобы избежать огромных аллокаций
            // на битом пакете.
            constexpr uint16_t MAX_REASONABLE_LEN =
                static_cast<uint16_t>(config::MAX_MESSAGE_LENGTH + 1024);
            if (header.messageLen > MAX_REASONABLE_LEN)
            {
                emit signalErrorOccurred(ErrorKind::Protocol,
                    QStringLiteral("Packet too large: %1 bytes").arg(header.messageLen));
                receiveBuffer_.clear();
                return;
            }

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
    catch (const std::bad_alloc&)
    {
        emit signalErrorOccurred(ErrorKind::Transport,
                                 QStringLiteral("Out of memory in receive buffer"));
        receiveBuffer_.clear();
    }
    catch (const std::exception& e)
    {
        emit signalErrorOccurred(ErrorKind::Transport,
                                 QString::fromUtf8(e.what()));
    }
}
