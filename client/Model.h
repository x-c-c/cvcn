#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QAbstractSocket>
#include <vector>
#include <cstdint>
#include "PacketData.h"
class Model : public QObject
{
    Q_OBJECT
public:
    explicit Model(QObject* parent = nullptr);
    ~Model() = default;

    void connectToServer(const QString& address, quint16 port);
    void sendRegRequest(const QString& username, const QString& password);
    void sendAuthRequest(const QString& username, const QString& password);

signals:
    void connected();
    void registrationFinished(bool success);
    void authFinished(bool success, uint32_t sessionID);
    void errorOccurred(const QString& errorString);

private slots:
    void slotConnected();
    void slotReadyRead();
    void slotSocketError(QAbstractSocket::SocketError error);

private:
    QTcpSocket* socket_;
    std::vector<uint8_t> receiveBuffer_;
    uint32_t messageID_ = 0;
    uint32_t sessionID_ = 0;

    void sendPacket(const std::vector<uint8_t>& packet);
    void increaseMessageID();
    void processIncomingPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
};
#endif // MODEL_H
