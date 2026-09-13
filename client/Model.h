#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QString>
#include <vector>
#include <cstdint>
#include "PacketData.h"

class Connection;

class Model : public QObject
{
    Q_OBJECT
public:
    explicit Model(QObject* parent = nullptr);
    ~Model() = default;

    void connectToServer(const QString& address, quint16 port);

    void sendRegRequest(const QString& username, const QString& password);
    void sendAuthRequest(const QString& username, const QString& password);
    void sendDeleteRequest(const QString& username, const QString& password);
    void sendMessage(uint32_t chatID, const QString& text);
    void sendFindUserRequest(const QString& query);
    void sendCreateChatRequest(const QString& peerUsername);
    void sendChatListRequest();

signals:
    void connected();
    void disconnected();
    void registrationFinished(bool success);
    void authFinished(bool success, uint32_t sessionID);
    void deleteFinished(bool success);
    void errorOccurred(const QString& errorString);
    void usersFound(const std::vector<std::string>& usernames);
    void chatCreated(bool success, uint32_t chatID, const QString& peerUsername);
    void chatListReceived(const std::vector<ChatListEntry>& chats);
    void messageReceived(uint32_t senderID,
                         const QString& senderUsername,
                         uint32_t chatID,
                         const QString& text);

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(const QString& errorString);
    void onRawPacketReceived(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);

private:
    Connection* connection_;
    uint32_t messageID_ = 0;
    uint32_t sessionID_ = 0;

    void sendPacket(const std::vector<uint8_t>& packet);
    void increaseMessageID();
    void processIncomingPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
};

#endif // MODEL_H
