#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <vector>
#include <cstdint>
#include "ErrorKind.h"
#include "PacketData.h"

/**
 * @file Connection.h
 * @brief Транспортный слой: владеет QTcpSocket, режет входящий поток на пакеты.
 *
 * Не знает о бизнес-логике. Эмитит signalRawPacketReceived для каждого полного пакета
 * и signalErrorOccurred(ErrorKind::Transport, ...) при сетевых сбоях.
 */
class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QObject* parent = nullptr);

    void connectToServer(const QString& address, quint16 port);
    void send(const std::vector<uint8_t>& packet);
    bool isConnected() const;

signals:
    void signalConnected();
    void signalDisconnected();
    void signalErrorOccurred(ErrorKind kind, const QString& errorString);
    void signalRawPacketReceived(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);

private slots:
    void slotConnected();
    void slotDisconnected();
    void slotReadyRead();
    void slotSocketError(QAbstractSocket::SocketError error);

private:
    QTcpSocket* socket_;
    std::vector<uint8_t> receiveBuffer_;
};
