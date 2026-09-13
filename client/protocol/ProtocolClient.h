#ifndef PROTOCOLCLIENT_H
#define PROTOCOLCLIENT_H

#include <QObject>
#include <QString>
#include <vector>
#include <cstdint>
#include "ErrorKind.h"
#include "PacketData.h"

class Connection;

/**
 * @file ProtocolClient.h
 * @brief Клиентская часть протокола: сериализация запросов и маршрутизация ответов.
 *
 * Не владеет Connection, получает его через конструктор. Это позволяет
 * подменить транспорт при тестировании.
 */
class ProtocolClient : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolClient(Connection* connection, QObject* parent = nullptr);
    ~ProtocolClient() = default;

    void connectToServer(const QString& address, quint16 port);

    void sendRegRequest(const QString& username, const QString& password);
    void sendAuthRequest(const QString& username, const QString& password);
    void sendDeleteRequest(const QString& username, const QString& password);
    void sendMessage(uint32_t chatID, const QString& text);
    void sendFindUserRequest(const QString& query);
    void sendCreateChatRequest(const QString& peerUsername);
    void sendChatListRequest();
    void sendDisconnectRequest();

signals:
    void signalConnected();
    void signalDisconnected();
    void signalRegistrationFinished(bool success);
    void signalAuthFinished(bool success, uint32_t sessionID);
    void signalDeleteFinished(bool success);
    void signalErrorOccurred(ErrorKind kind, const QString& errorString);
    void signalUsersFound(const std::vector<std::string>& usernames);
    void signalChatCreated(bool success, uint32_t chatID, const QString& peerUsername);
    void signalChatListReceived(const std::vector<ChatListEntry>& chats);
    void signalMessageReceived(const QString& senderUsername,
                               uint32_t chatID,
                               const QString& text);

private slots:
    void slotConnected();
    void slotDisconnected();
    void slotTransportError(ErrorKind kind, const QString& errorString);
    void slotRawPacketReceived(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);

private:
    Connection* connection_;
    uint32_t messageID_ = 0;
    uint32_t sessionID_ = 0;

    void sendPacket(const std::vector<uint8_t>& packet);
    void increaseMessageID();
    void processIncomingPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
};

#endif // PROTOCOLCLIENT_H
