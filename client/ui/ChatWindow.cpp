#include "ChatWindow.h"
#include "ui_ChatWindow.h"
#include "AppConfig.h"
#include <QMessageBox>
#include <QTime>

ChatWindow::ChatWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::ChatWindow)
{
    ui->setupUi(this);

    connect(ui->sendPushButton, &QPushButton::clicked, this, &ChatWindow::slotClickedSendButton);
    connect(ui->messageLineEdit, &QLineEdit::returnPressed, this, &ChatWindow::slotClickedSendButton);
    connect(ui->findPushButton, &QPushButton::clicked, this, &ChatWindow::slotClickedFindButton);
    connect(ui->chatsListWidget, &QListWidget::itemSelectionChanged, this, &ChatWindow::slotChatSelectionChanged);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::appendMessageInHistory(const QString& sender, const QString& text)
{
    const QString time = QTime::currentTime()
        .toString(QString::fromUtf8(config::CHAT_TIME_FORMAT));
    ui->messagesTextBrowser->append(QStringLiteral("[%1] %2: %3").arg(time, sender, text));
}

void ChatWindow::showInformation(const QString& title, const QString& text)
{
    QMessageBox::information(this, title, text);
}

void ChatWindow::showWarning(const QString& title, const QString& text)
{
    QMessageBox::warning(this, title, text);
}

void ChatWindow::slotClickedSendButton()
{
    if (currentChatID_ == 0)
    {
        appendMessageInHistory(QStringLiteral("system"),
                               QStringLiteral("Select a chat first"));
        return;
    }

    const QString text = ui->messageLineEdit->text().trimmed();
    if (text.isEmpty())
        return;

    ui->messageLineEdit->clear();
    emit signalMessageSendRequested(currentChatID_, text);
}

void ChatWindow::slotClickedFindButton()
{
    const QString query = ui->searchLineEdit->text().trimmed();
    if (query.isEmpty())
        return;
    emit signalFindUserRequested(query);
}

void ChatWindow::slotChatSelectionChanged()
{
    auto* item = ui->chatsListWidget->currentItem();
    if (!item)
        return;
    const uint32_t id = item->data(Qt::UserRole).toUInt();
    currentChatID_ = id;
    ui->messagesTextBrowser->clear();
    emit signalChatSelected(id);
}

void ChatWindow::setChatList(const std::vector<ChatListEntry>& chats)
{
    ui->chatsListWidget->clear();
    for (const auto& e : chats)
        addChat(e.chatID, QString::fromStdString(e.peerUsername));
}

void ChatWindow::addChat(uint32_t chatID, const QString& peerUsername)
{
    for (int i = 0; i < ui->chatsListWidget->count(); ++i)
    {
        auto* existing = ui->chatsListWidget->item(i);
        if (existing->data(Qt::UserRole).toUInt() == chatID)
            return;
    }
    auto* item = new QListWidgetItem(peerUsername);
    item->setData(Qt::UserRole, chatID);
    ui->chatsListWidget->addItem(item);
}

void ChatWindow::appendIncomingMessage(uint32_t chatID, const QString& sender, const QString& text)
{
    if (chatID == currentChatID_)
        appendMessageInHistory(sender, text);
}
