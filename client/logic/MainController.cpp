#include "MainController.h"
#include <QDebug>
#include "Validator.h"
#include "Logger.h"
#include <QInputDialog>
#include <QMessageBox>
MainController::MainController(ProtocolClient& protocolClient, AccountDialog& view, ChatWindow& chatWindow):
    QObject(nullptr), protocolClient_(protocolClient), view_(view), chatWindow_(chatWindow)
{
    connect(&view_, &AccountDialog::signalAuthRequested, this, &MainController::slotAuthRequested);
    connect(&view_, &AccountDialog::signalRegRequested,  this, &MainController::slotRegRequested);
    connect(&view_, &AccountDialog::signalDelRequested,  this, &MainController::slotDelRequested);

    connect(&chatWindow_, &ChatWindow::signalMessageSendRequested, this, &MainController::slotMessageSendRequested);
    connect(&protocolClient_, &ProtocolClient::registrationFinished, this, &MainController::onRegistrationFinished);
    connect(&protocolClient_, &ProtocolClient::authFinished,         this, &MainController::onAuthFinished);
    connect(&protocolClient_, &ProtocolClient::deleteFinished,       this, &MainController::onDeleteFinished);
    connect(&protocolClient_, &ProtocolClient::errorOccurred,        this, &MainController::onError);
    connect(&chatWindow_, &ChatWindow::signalFindUserRequested, this, &MainController::slotFindUserRequested);
    connect(&chatWindow_, &ChatWindow::signalCreateChatRequested, this, &MainController::slotCreateChatRequested);
    connect(&chatWindow_, &ChatWindow::signalChatSelected, this, &MainController::slotChatSelected);

    connect(&protocolClient_, &ProtocolClient::usersFound, this, &MainController::onUsersFound);
    connect(&protocolClient_, &ProtocolClient::chatCreated, this, &MainController::onChatCreated);
    connect(&protocolClient_, &ProtocolClient::chatListReceived, this, &MainController::onChatListReceived);


    connect(&protocolClient_, &ProtocolClient::messageReceived, this, &MainController::onMessageReceived);
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

void MainController::onRegistrationFinished(bool success)
{
    LOG_INFO("Registration {}", success ? "OK" : "FAILED");
}

void MainController::onAuthFinished(bool success, uint32_t sessionID)
{
    LOG_INFO("Auth {} sessionID = {}", success ? "OK" : "FAILED", sessionID);
    if (success)
    {
        chatWindow_.setCurrentUser(currentUsername_);
        view_.hide();
        chatWindow_.show();
        protocolClient_.sendChatListRequest();
    }
}

void MainController::onDeleteFinished(bool success)
{
    LOG_INFO("Delete {}", success ? "OK" : "FAILED");
}

void MainController::onError(const QString& errorString)
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

void MainController::onUsersFound(const std::vector<std::string>& usernames)
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

void MainController::onChatCreated(bool success, uint32_t chatID, const QString& peerUsername)
{
    if (success)
        chatWindow_.addChat(chatID, peerUsername);
    else
        QMessageBox::warning(&chatWindow_, "Chat", "Failed to create chat");
}

void MainController::onChatListReceived(const std::vector<ChatListEntry>& chats)
{
    chatWindow_.setChatList(chats);
}

void MainController::onMessageReceived(const QString& senderUsername,
                                   uint32_t chatID,
                                   const QString& text)
{
    chatWindow_.appendIncomingMessage(chatID, senderUsername, text);
}



