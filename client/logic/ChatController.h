#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QObject>
#include <QString>
#include <vector>
#include <cstdint>
#include "PacketData.h"

class ProtocolClient;
class ChatWindow;

/**
 * @file ChatController.h
 * @brief Список чатов: поиск пользователей, создание 1:1 чата, выбор.
 */
class ChatController : public QObject
{
    Q_OBJECT
public:
    ChatController(ProtocolClient& protocolClient,
                   ChatWindow& chatWindow,
                   QObject* parent = nullptr);

    /** @brief Запросить список чатов у сервера. Вызывается после успешного входа. */
    void loadChatList();

signals:
    void signalShowInformation(const QString& title, const QString& text);
    void signalShowWarning(const QString& title, const QString& text);

private slots:
    void slotFindUserRequested(const QString& query);
    void slotCreateChatRequested(const QString& peerUsername);
    void slotChatSelected(uint32_t chatID);

    void slotUsersFound(const std::vector<std::string>& usernames);
    void slotChatCreated(bool success, uint32_t chatID, const QString& peerUsername);
    void slotChatListReceived(const std::vector<ChatListEntry>& chats);

private:
    ProtocolClient& protocolClient_;
    ChatWindow& chatWindow_;
};

#endif // CHATCONTROLLER_H
