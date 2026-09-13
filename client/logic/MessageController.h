#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <QObject>
#include <QString>
#include <cstdint>

class ProtocolClient;
class ChatWindow;
class Session;

/**
 * @file MessageController.h
 * @brief Отправка и приём сообщений.
 *
 * Валидирует текст, показывает своё сообщение в истории (сразу после отправки),
 * а входящие сообщения передаёт в ChatWindow.
 */
class MessageController : public QObject
{
    Q_OBJECT
public:
    MessageController(ProtocolClient& protocolClient,
                      ChatWindow& chatWindow,
                      Session& session,
                      QObject* parent = nullptr);

signals:
    void signalShowWarning(const QString& title, const QString& text);

private slots:
    void slotMessageSendRequested(uint32_t chatID, const QString& text);
    void slotMessageReceived(const QString& senderUsername,
                             uint32_t chatID,
                             const QString& text);

private:
    ProtocolClient& protocolClient_;
    ChatWindow& chatWindow_;
    Session& session_;
};

#endif // MESSAGECONTROLLER_H
