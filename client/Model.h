#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QAbstractSocket>
class Model: public QObject
{
    Q_OBJECT
public:
   explicit Model(QObject* parent = nullptr);
    ~Model() = default;
    void connectToServer(const QString& address, const quint16 port);
    void sendAuthRequest(const QString& username, const QString& password);
private:
    QTcpSocket* socket_;

private slots:
    void slotConnected();
    void slotSocketError(QAbstractSocket::SocketError error);
};
#endif // MODEL_H
