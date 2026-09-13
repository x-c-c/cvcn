#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <vector>
#include <cstdint>
#include "PacketData.h"
class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QObject* parent = nullptr);

    void connectToServer(const QString& address, quint16 port);
    void send(const std::vector<uint8_t>& packet);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& errorString);
    void rawPacketReceived(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);

private slots:
    void slotConnected();
    void slotDisconnected();
    void slotReadyRead();
    void slotSocketError(QAbstractSocket::SocketError error);

private:
    QTcpSocket* socket_;
    std::vector<uint8_t> receiveBuffer_;
};
