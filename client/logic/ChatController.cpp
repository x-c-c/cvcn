#include "ChatController.h"
#include "ProtocolClient.h"
#include "ChatWindow.h"
#include "Logger.h"
#include <QInputDialog>

ChatController::ChatController(ProtocolClient& protocolClient,
                               ChatWindow& chatWindow,
                               QObject* parent):
    QObject(parent),
    protocolClient_(protocolClient),
    chatWindow_(chatWindow)
{
    connect(&chatWindow_, &ChatWindow::signalFindUserRequested,   this, &ChatController::slotFindUserRequested);
    connect(&chatWindow_, &ChatWindow::signalCreateChatRequested, this, &ChatController::slotCreateChatRequested);
    connect(&chatWindow_, &ChatWindow::signalChatSelected,        this, &ChatController::slotChatSelected);

    connect(&protocolClient_, &ProtocolClient::signalUsersFound,       this, &ChatController::slotUsersFound);
    connect(&protocolClient_, &ProtocolClient::signalChatCreated,      this, &ChatController::slotChatCreated);
    connect(&protocolClient_, &ProtocolClient::signalChatListReceived, this, &ChatController::slotChatListReceived);
}

void ChatController::loadChatList()
{
    protocolClient_.sendChatListRequest();
}

void ChatController::slotFindUserRequested(const QString& query)
{
    protocolClient_.sendFindUserRequest(query);
}

void ChatController::slotCreateChatRequested(const QString& peerUsername)
{
    protocolClient_.sendCreateChatRequest(peerUsername);
}

void ChatController::slotChatSelected(uint32_t chatID)
{
    Q_UNUSED(chatID);
    // В следующей итерации: запрос истории сообщений
}

void ChatController::slotUsersFound(const std::vector<std::string>& usernames)
{
    if (usernames.empty())
    {
        emit signalShowInformation(QStringLiteral("Search"),
                                   QStringLiteral("No users found"));
        return;
    }

    QStringList list;
    for (const auto& u : usernames)
        list << QString::fromStdString(u);

    bool ok = false;
    const QString chosen = QInputDialog::getItem(&chatWindow_,
                                                 QStringLiteral("Found users"),
                                                 QStringLiteral("Select user:"),
                                                 list, 0, false, &ok);
    if (ok && !chosen.isEmpty())
        protocolClient_.sendCreateChatRequest(chosen);
}

void ChatController::slotChatCreated(bool success, uint32_t chatID, const QString& peerUsername)
{
    if (success)
        chatWindow_.addChat(chatID, peerUsername);
    else
        emit signalShowWarning(QStringLiteral("Chat"),
                               QStringLiteral("Failed to create chat"));
}

void ChatController::slotChatListReceived(const std::vector<ChatListEntry>& chats)
{
    chatWindow_.setChatList(chats);
}
