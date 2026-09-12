#include "Controller.h"
#include <QDebug>
#include "Validator.h"
#include <QInputDialog>
#include <QMessageBox>
Controller::Controller(Model& model, AccountDialog& view, ChatWindow& chatWindow):
    QObject(nullptr), model_(model), view_(view), chatWindow_(chatWindow)
{
    connect(&view_, &AccountDialog::signalAuthRequested, this, &Controller::slotAuthRequested);
    connect(&view_, &AccountDialog::signalRegRequested,  this, &Controller::slotRegRequested);
    connect(&view_, &AccountDialog::signalDelRequested,  this, &Controller::slotDelRequested);

    connect(&chatWindow_, &ChatWindow::signalMessageSendRequested, this, &Controller::slotMessageSendRequested);
    connect(&model_, &Model::registrationFinished, this, &Controller::onRegistrationFinished);
    connect(&model_, &Model::authFinished,         this, &Controller::onAuthFinished);
    connect(&model_, &Model::deleteFinished,       this, &Controller::onDeleteFinished);
    connect(&model_, &Model::errorOccurred,        this, &Controller::onError);
    connect(&chatWindow_, &ChatWindow::signalFindUserRequested, this, &Controller::slotFindUserRequested);
    connect(&chatWindow_, &ChatWindow::signalCreateChatRequested, this, &Controller::slotCreateChatRequested);
    connect(&chatWindow_, &ChatWindow::signalChatSelected, this, &Controller::slotChatSelected);

    connect(&model_, &Model::usersFound, this, &Controller::onUsersFound);
    connect(&model_, &Model::chatCreated, this, &Controller::onChatCreated);
    connect(&model_, &Model::chatListReceived, this, &Controller::onChatListReceived);
}

void Controller::connectToServer(const QString& host, quint16 port)
{
    model_.connectToServer(host, port);
}

void Controller::slotAuthRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        qWarning() << "Auth rejected locally: invalid username or password format";
        return;
    }
    currentUsername_ = username;
    model_.sendAuthRequest(username, password);
}

void Controller::slotRegRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        qWarning() << "Register rejected locally: invalid username or password format";
        return;
    }
    model_.sendRegRequest(username, password);
}

void Controller::slotDelRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        qWarning() << "Delete rejected locally: invalid username or password format";
        return;
    }
    model_.sendDeleteRequest(username, password);
}

void Controller::slotMessageSendRequested(const QString& text)
{
    // chatID = 0 — заглушка, пока нет списка чатов
    model_.sendMessage(0, text);
}

void Controller::onRegistrationFinished(bool success)
{
    qDebug() << "Registration" << (success ? "OK" : "FAILED");
}

void Controller::onAuthFinished(bool success, uint32_t sessionID)
{
    qDebug() << "Auth" << (success ? "OK" : "FAILED") << "sessionID =" << sessionID;
    if (success)
    {
        chatWindow_.setCurrentUser(currentUsername_);
        view_.hide();
        chatWindow_.show();
        model_.sendChatListRequest();
    }
}

void Controller::onDeleteFinished(bool success)
{
    qDebug() << "Delete" << (success ? "OK" : "FAILED");
}

void Controller::onError(const QString& errorString)
{
    qDebug() << "Error:" << errorString;
}
void Controller::slotFindUserRequested(const QString& query)
{
    model_.sendFindUserRequest(query);
}

void Controller::slotCreateChatRequested(const QString& peerUsername)
{
    model_.sendCreateChatRequest(peerUsername);
}

void Controller::slotChatSelected(uint32_t chatID)
{
    Q_UNUSED(chatID);
    // В следующей итерации: запрос истории сообщений
}

void Controller::onUsersFound(const std::vector<std::string>& usernames)
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
        model_.sendCreateChatRequest(chosen);
}

void Controller::onChatCreated(bool success, uint32_t chatID, const QString& peerUsername)
{
    if (success)
        chatWindow_.addChat(chatID, peerUsername);
    else
        QMessageBox::warning(&chatWindow_, "Chat", "Failed to create chat");
}

void Controller::onChatListReceived(const std::vector<ChatListEntry>& chats)
{
    chatWindow_.setChatList(chats);
}





