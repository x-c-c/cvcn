#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H
#include <QObject>
#include "ProtocolClient.h"
#include "AccountDialog.h"
#include "ChatWindow.h"
class MainController : public QObject
{
    Q_OBJECT
public:
    MainController(ProtocolClient& protocolClient, AccountDialog& accountDialog, ChatWindow& chatWindow);
    void connectToServer(const QString& host, quint16 port);

private:
    ProtocolClient& protocolClient_;
    AccountDialog& accountDialog_;
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
    void slotUsersFound(const std::vector<std::string>& usernames);
    void slotChatCreated(bool success, uint32_t chatID, const QString& peerUsername);
    void slotChatListReceived(const std::vector<ChatListEntry>& chats);

    void slotRegistrationFinished(bool success);
    void slotAuthFinished(bool success, uint32_t sessionID);
    void slotDeleteFinished(bool success);
    void slotError(const QString& errorString);

    void slotMessageReceived(const QString& senderUsername,
                           uint32_t chatID,
                           const QString& text);
};

#endif // MAINCONTROLLER_H
