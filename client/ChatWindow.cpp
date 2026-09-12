#include "ChatWindow.h"
#include "ui_ChatWindow.h"
#include "Validator.h"
#include <QTime>

ChatWindow::ChatWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::ChatWindow)
{
    ui->setupUi(this);

    connect(ui->sendPushButton, &QPushButton::clicked, this, &ChatWindow::slotClickedSendButton);
    connect(ui->messageLineEdit, &QLineEdit::returnPressed, this, &ChatWindow::slotClickedSendButton);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::setCurrentUser(const QString& username)
{
    currentUser_ = username;
}

void ChatWindow::appendMessageInHistory(const QString& sender, const QString& text)
{
    const QString time = QTime::currentTime().toString(QStringLiteral("HH:mm"));
    ui->messagesTextBrowser->append(QStringLiteral("[%1] %2: %3").arg(time, sender, text));
}

void ChatWindow::slotClickedSendButton()
{
    const QString text = ui->messageLineEdit->text().trimmed();
    if (text.isEmpty())
    {
        return;
    }
    if (!Validator::validateMessage(text.toStdString()))
    {
        appendMessageInHistory(QStringLiteral("system"), QStringLiteral("Message rejected locally: invalid format"));
        return;
    }

    ui->messageLineEdit->clear();

    const QString displayName = currentUser_.isEmpty() ? QStringLiteral("me") : currentUser_;

    appendMessageInHistory(displayName, text);
    emit signalMessageSendRequested(text);
}
