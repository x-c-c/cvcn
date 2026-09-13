#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H
#include <QObject>
#include "ProtocolClient.h"
#include "AccountDialog.h"
#include "ChatWindow.h"
#include "Session.h"
#include "AuthController.h"
#include "ChatController.h"
#include "MessageController.h"
#include "ErrorKind.h"

/**
 * @file MainController.h
 * @brief Координатор клиента.
 *
 * Создаёт Session и три специализированных контроллера. Сам занимается
 * только тем, что не относится ни к одному из доменов: переключением окон,
 * реакцией на подключение/отключение и завершение работы, глобальными
 * ошибками транспорта и протокола.
 */
class MainController : public QObject
{
    Q_OBJECT
public:
    MainController(ProtocolClient& protocolClient,
                   AccountDialog& accountDialog,
                   ChatWindow& chatWindow);
    void connectToServer(const QString& host, quint16 port);

public slots:
    /** @brief Отправить DisconnectRequest перед выходом. */
    void slotAboutToQuit();

private slots:
    void slotAuthenticationSucceeded();
    void slotError(ErrorKind kind, const QString& errorString);
    void slotShowInformation(const QString& title, const QString& text);
    void slotShowWarning(const QString& title, const QString& text);
    void slotConnected();
    void slotDisconnected();

private:
    ProtocolClient& protocolClient_;
    AccountDialog& accountDialog_;
    ChatWindow& chatWindow_;

    Session session_;
    AuthController authController_;
    ChatController chatController_;
    MessageController messageController_;
};

#endif // MAINCONTROLLER_H
