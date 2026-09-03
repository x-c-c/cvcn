#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QAbstractSocket>
#include <vector>
#include <cstdint>
class Model: public QObject
{
    Q_OBJECT
public:
   explicit Model(QObject* parent = nullptr);
    ~Model() = default;
    void connectToServer(const QString& address, const quint16 port);
    void sendRegRequest(const QString& username, const QString& password);
    void sendPacket(const std::vector<uint8_t>& packet);
private:
    QTcpSocket* socket_;
    uint32_t messageID_ = 0;
    uint32_t sessionID_ = 0;
    vector<uint8_t> receiveBuffer_ = {};
    void increaseMessageID();
private slots:
    void slotConnected();
    void slotReadyRead();
    void slotSocketError(QAbstractSocket::SocketError error);
};
#endif // MODEL_H
