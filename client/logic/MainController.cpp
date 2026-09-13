#include "MainController.h"
#include "Validator.h"
#include "Logger.h"
#include <QInputDialog>
#include <QMessageBox>
MainController::MainController(ProtocolClient& protocolClient, AccountDialog& accountDialog, ChatWindow& chatWindow):
    QObject(nullptr), protocolClient_(protocolClient), accountDialog_(accountDialog), chatWindow_(chatWindow)
{
    connect(&accountDialog_, &AccountDialog::signalAuthRequested, this, &MainController::slotAuthRequested);
    connect(&accountDialog_, &AccountDialog::signalRegRequested,  this, &MainController::slotRegRequested);
    connect(&accountDialog_, &AccountDialog::signalDelRequested,  this, &MainController::slotDelRequested);

    connect(&chatWindow_, &ChatWindow::signalMessageSendRequested, this, &MainController::slotMessageSendRequested);
    connect(&protocolClient_, &ProtocolClient::signalRegistrationFinished, this, &MainController::slotRegistrationFinished);
    connect(&protocolClient_, &ProtocolClient::signalAuthFinished,         this, &MainController::slotAuthFinished);
    connect(&protocolClient_, &ProtocolClient::signalDeleteFinished,       this, &MainController::slotDeleteFinished);
    connect(&protocolClient_, &ProtocolClient::signalErrorOccurred,        this, &MainController::slotError);
    connect(&chatWindow_, &ChatWindow::signalFindUserRequested, this, &MainController::slotFindUserRequested);
    connect(&chatWindow_, &ChatWindow::signalCreateChatRequested, this, &MainController::slotCreateChatRequested);
    connect(&chatWindow_, &ChatWindow::signalChatSelected, this, &MainController::slotChatSelected);

    connect(&protocolClient_, &ProtocolClient::signalUsersFound, this, &MainController::slotUsersFound);
    connect(&protocolClient_, &ProtocolClient::signalChatCreated, this, &MainController::slotChatCreated);
    connect(&protocolClient_, &ProtocolClient::signalChatListReceived, this, &MainController::slotChatListReceived);


    connect(&protocolClient_, &ProtocolClient::signalMessageReceived, this, &MainController::slotMessageReceived);
}

void MainController::connectToServer(const QString& host, quint16 port)
{
    protocolClient_.connectToServer(host, port);
}

void MainController::slotAuthRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Auth rejected locally: invalid username or password format");
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
        chatWindow_.setCurrentUser(currentUsername_);
        accountDialog_.hide();
        chatWindow_.show();
        protocolClient_.sendChatListRequest();
    }
}

void MainController::slotDeleteFinished(bool success)
{
    LOG_INFO("Delete {}", success ? "OK" : "FAILED");
}

void MainController::slotError(const QString& errorString)
{
    LOG_ERROR("{}", errorString.toStdString());
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
        QMessageBox::information(&chatWindow_, "Search", "No users found");
        return;
    }
    QStringList list;
    for (const auto& u : usernames)
        list << QString::fromStdString(u);
    bool ok = false;
    const QString chosen = QInputDialog::getItem(&chatWindow_, "Found users",
                                                 "Select user:", list, 0, false, &ok);
    if (ok && !chosen.isEmpty())
        protocolClient_.sendCreateChatRequest(chosen);
}

void MainController::slotChatCreated(bool success, uint32_t chatID, const QString& peerUsername)
{
    if (success)
        chatWindow_.addChat(chatID, peerUsername);
    else
        QMessageBox::warning(&chatWindow_, "Chat", "Failed to create chat");
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



