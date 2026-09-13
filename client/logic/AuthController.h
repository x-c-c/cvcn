#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include <QObject>
#include <QString>
#include <cstdint>

class ProtocolClient;
class AccountDialog;
class Session;

/**
 * @file AuthController.h
 * @brief Регистрация, вход, удаление аккаунта.
 *
 * Обрабатывает сигналы AccountDialog (намерения пользователя) и ответы
 * ProtocolClient. Обновляет Session. Эмитит signalAuthenticationSucceeded,
 * чтобы MainController переключил окна.
 */
class AuthController : public QObject
{
    Q_OBJECT
public:
    AuthController(ProtocolClient& protocolClient,
                   AccountDialog& accountDialog,
                   Session& session,
                   QObject* parent = nullptr);

signals:
    /** @brief Пользователь успешно вошёл. MainController переключает окна. */
    void signalAuthenticationSucceeded();

    /** @brief Показать пользователю предупреждение. */
    void signalShowWarning(const QString& title, const QString& text);

private slots:
    void slotAuthRequested(const QString& username, const QString& password);
    void slotRegRequested(const QString& username, const QString& password);
    void slotDelRequested(const QString& username, const QString& password);

    void slotRegistrationFinished(bool success);
    void slotAuthFinished(bool success, uint32_t sessionID);
    void slotDeleteFinished(bool success);

private:
    ProtocolClient& protocolClient_;
    AccountDialog& accountDialog_;
    Session& session_;

    /** @brief Имя из последнего запроса авторизации, чтобы записать его в Session при успехе. */
    QString pendingUsername_;
};

#endif // AUTHCONTROLLER_H
