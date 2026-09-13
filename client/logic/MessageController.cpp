#include "MessageController.h"
#include "ProtocolClient.h"
#include "ChatWindow.h"
#include "Session.h"
#include "Validator.h"
#include "Logger.h"

MessageController::MessageController(ProtocolClient& protocolClient,
                                     ChatWindow& chatWindow,
                                     Session& session,
                                     QObject* parent):
    QObject(parent),
    protocolClient_(protocolClient),
    chatWindow_(chatWindow),
    session_(session)
{
    connect(&chatWindow_, &ChatWindow::signalMessageSendRequested, this, &MessageController::slotMessageSendRequested);
    connect(&protocolClient_, &ProtocolClient::signalMessageReceived, this, &MessageController::slotMessageReceived);
}

void MessageController::slotMessageSendRequested(uint32_t chatID, const QString& text)
{
    if (!Validator::validateMessage(text.toStdString()))
    {
        LOG_WARN("Message rejected locally: invalid format");
        emit signalShowWarning(QStringLiteral("Message"),
                               QStringLiteral("Message rejected: invalid format"));
        return;
    }

    chatWindow_.appendMessageInHistory(session_.username(), text);
    protocolClient_.sendMessage(chatID, text);
}

void MessageController::slotMessageReceived(const QString& senderUsername,
                                            uint32_t chatID,
                                            const QString& text)
{
    chatWindow_.appendIncomingMessage(chatID, senderUsername, text);
}
