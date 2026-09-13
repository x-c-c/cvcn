#include "MainController.h"
#include "Validator.h"
#include "Logger.h"
#include <QInputDialog>

MainController::MainController(ProtocolClient& protocolClient,
                               AccountDialog& accountDialog,
                               ChatWindow& chatWindow):
    QObject(nullptr),
    protocolClient_(protocolClient),
    accountDialog_(accountDialog),
    chatWindow_(chatWindow)
{
    connect(&accountDialog_, &AccountDialog::signalAuthRequested, this, &MainController::slotAuthRequested);
    connect(&accountDialog_, &AccountDialog::signalRegRequested,  this, &MainController::slotRegRequested);
    connect(&accountDialog_, &AccountDialog::signalDelRequested,  this, &MainController::slotDelRequested);

    connect(&chatWindow_, &ChatWindow::signalMessageSendRequested, this, &MainController::slotMessageSendRequested);
    connect(&chatWindow_, &ChatWindow::signalFindUserRequested,    this, &MainController::slotFindUserRequested);
    connect(&chatWindow_, &ChatWindow::signalCreateChatRequested,  this, &MainController::slotCreateChatRequested);
    connect(&chatWindow_, &ChatWindow::signalChatSelected,         this, &MainController::slotChatSelected);

    connect(&protocolClient_, &ProtocolClient::signalRegistrationFinished, this, &MainController::slotRegistrationFinished);
    connect(&protocolClient_, &ProtocolClient::signalAuthFinished,         this, &MainController::slotAuthFinished);
    connect(&protocolClient_, &ProtocolClient::signalDeleteFinished,       this, &MainController::slotDeleteFinished);
    connect(&protocolClient_, &ProtocolClient::signalErrorOccurred,        this, &MainController::slotError);
    connect(&protocolClient_, &ProtocolClient::signalUsersFound,           this, &MainController::slotUsersFound);
    connect(&protocolClient_, &ProtocolClient::signalChatCreated,          this, &MainController::slotChatCreated);
    connect(&protocolClient_, &ProtocolClient::signalChatListReceived,     this, &MainController::slotChatListReceived);
    connect(&protocolClient_, &ProtocolClient::signalMessageReceived,      this, &MainController::slotMessageReceived);
}

void MainController::connectToServer(const QString& host, quint16 port)
{
    protocolClient_.connectToServer(host, port);
}

void MainController::slotAboutToQuit()
{
    LOG_INFO("Client shutting down, sending DisconnectRequest");
    protocolClient_.sendDisconnectRequest();
}

void MainController::slotAuthRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Auth rejected locally: invalid username or password format");
        chatWindow_.showWarning(QStringLiteral("Auth"),
                                QStringLiteral("Invalid username or password format"));
        return;
    }
    currentUsername_ = username;
    protocolClient_.sendAuthRequest(username, password);
}

void MainController::slotRegRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Register rejected locally: invalid username or password format");
        return;
    }
    protocolClient_.sendRegRequest(username, password);
}

void MainController::slotDelRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Delete rejected locally: invalid username or password format");
        return;
    }
    protocolClient_.sendDeleteRequest(username, password);
}

void MainController::slotMessageSendRequested(uint32_t chatID, const QString& text)
{
    if (!Validator::validateMessage(text.toStdString()))
    {
        LOG_WARN("Message rejected locally: invalid format");
        chatWindow_.showWarning(QStringLiteral("Message"),
                                QStringLiteral("Message rejected: invalid format"));
        return;
    }

    chatWindow_.appendMessageInHistory(currentUsername_, text);
    protocolClient_.sendMessage(chatID, text);
}

void MainController::slotRegistrationFinished(bool success)
{
    LOG_INFO("Registration {}", success ? "OK" : "FAILED");
}

void MainController::slotAuthFinished(bool success, uint32_t sessionID)
{
    LOG_INFO("Auth {} sessionID = {}", success ? "OK" : "FAILED", sessionID);
    if (success)
    {
        accountDialog_.hide();
        chatWindow_.show();
        protocolClient_.sendChatListRequest();
    }
}

void MainController::slotDeleteFinished(bool success)
{
    LOG_INFO("Delete {}", success ? "OK" : "FAILED");
}

void MainController::slotError(ErrorKind kind, const QString& errorString)
{
    switch (kind)
    {
    case ErrorKind::Transport:
        LOG_ERROR("Transport error: {}", errorString.toStdString());
        break;
    case ErrorKind::Protocol:
        LOG_ERROR("Protocol error: {}", errorString.toStdString());
        break;
    case ErrorKind::Business:
        LOG_WARN("Business error: {}", errorString.toStdString());
        break;
    }
}

void MainController::slotFindUserRequested(const QString& query)
{
    protocolClient_.sendFindUserRequest(query);
}

void MainController::slotCreateChatRequested(const QString& peerUsername)
{
    protocolClient_.sendCreateChatRequest(peerUsername);
}

void MainController::slotChatSelected(uint32_t chatID)
{
    Q_UNUSED(chatID);
    // В следующей итерации: запрос истории сообщений
}

void MainController::slotUsersFound(const std::vector<std::string>& usernames)
{
    if (usernames.empty())
    {
        chatWindow_.showInformation(QStringLiteral("Search"),
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

void MainController::slotChatCreated(bool success, uint32_t chatID, const QString& peerUsername)
{
    if (success)
        chatWindow_.addChat(chatID, peerUsername);
    else
        chatWindow_.showWarning(QStringLiteral("Chat"),
                                QStringLiteral("Failed to create chat"));
}

void MainController::slotChatListReceived(const std::vector<ChatListEntry>& chats)
{
    chatWindow_.setChatList(chats);
}

void MainController::slotMessageReceived(const QString& senderUsername,
                                         uint32_t chatID,
                                         const QString& text)
{
    chatWindow_.appendIncomingMessage(chatID, senderUsername, text);
}
