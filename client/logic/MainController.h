#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include "ProtocolClient.h"
#include "AccountDialog.h"
#include "ChatWindow.h"
class MainController : public QObject
{
    Q_OBJECT
public:
    MainController(ProtocolClient& protocolClient, AccountDialog& view, ChatWindow& chatWindow);
    void connectToServer(const QString& host, quint16 port);

private:
    ProtocolClient& protocolClient_;
    AccountDialog& view_;
    ChatWindow& chatWindow_;
    QString currentUsername_;

private slots:
    void slotAuthRequested(const QString& username, const QString& password);
    void slotRegRequested(const QString& username, const QString& password);
    void slotDelRequested(const QString& username, const QString& password);
    void slotMessageSendRequested(uint32_t chatID, const QString& text);
    void slotFindUserRequested(const QString& query);
    void slotCreateChatRequested(const QString& peerUsername);
    void slotChatSelected(uint32_t chatID);
    void onUsersFound(const std::vector<std::string>& usernames);
    void onChatCreated(bool success, uint32_t chatID, const QString& peerUsername);
    void onChatListReceived(const std::vector<ChatListEntry>& chats);

    void onRegistrationFinished(bool success);
    void onAuthFinished(bool success, uint32_t sessionID);
    void onDeleteFinished(bool success);
    void onError(const QString& errorString);

    void onMessageReceived(const QString& senderUsername,
                           uint32_t chatID,
                           const QString& text);
};

#endif // CONTROLLER_H
