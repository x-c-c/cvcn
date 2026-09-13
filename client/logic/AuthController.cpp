#include "AuthController.h"
#include "ProtocolClient.h"
#include "AccountDialog.h"
#include "Session.h"
#include "Validator.h"
#include "Logger.h"

AuthController::AuthController(ProtocolClient& protocolClient,
                               AccountDialog& accountDialog,
                               Session& session,
                               QObject* parent):
    QObject(parent),
    protocolClient_(protocolClient),
    accountDialog_(accountDialog),
    session_(session)
{
    connect(&accountDialog_, &AccountDialog::signalAuthRequested, this, &AuthController::slotAuthRequested);
    connect(&accountDialog_, &AccountDialog::signalRegRequested,  this, &AuthController::slotRegRequested);
    connect(&accountDialog_, &AccountDialog::signalDelRequested,  this, &AuthController::slotDelRequested);

    connect(&protocolClient_, &ProtocolClient::signalRegistrationFinished, this, &AuthController::slotRegistrationFinished);
    connect(&protocolClient_, &ProtocolClient::signalAuthFinished,         this, &AuthController::slotAuthFinished);
    connect(&protocolClient_, &ProtocolClient::signalDeleteFinished,       this, &AuthController::slotDeleteFinished);
}

void AuthController::slotAuthRequested(const QString& username, const QString& password)
{
    if (session_.state() == AppState::Authenticating)
    {
        LOG_WARN("Auth already in progress, ignoring request");
        return;
    }

    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Auth rejected locally: invalid username or password format");
        emit signalShowWarning(QStringLiteral("Auth"),
                               QStringLiteral("Invalid username or password format"));
        return;
    }

    pendingUsername_ = username;
    session_.setState(AppState::Authenticating);
    protocolClient_.sendAuthRequest(username, password);
}

void AuthController::slotRegRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Register rejected locally: invalid username or password format");
        emit signalShowWarning(QStringLiteral("Register"),
                               QStringLiteral("Invalid username or password format"));
        return;
    }
    protocolClient_.sendRegRequest(username, password);
}

void AuthController::slotDelRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Delete rejected locally: invalid username or password format");
        emit signalShowWarning(QStringLiteral("Delete"),
                               QStringLiteral("Invalid username or password format"));
        return;
    }
    protocolClient_.sendDeleteRequest(username, password);
}

void AuthController::slotRegistrationFinished(bool success)
{
    LOG_INFO("Registration {}", success ? "OK" : "FAILED");
}

void AuthController::slotAuthFinished(bool success, uint32_t sessionID)
{
    LOG_INFO("Auth {} sessionID = {}", success ? "OK" : "FAILED", sessionID);
    if (success)
    {
        session_.setUsername(pendingUsername_);
        session_.setState(AppState::Authenticated);
        emit signalAuthenticationSucceeded();
    }
    else
    {
        session_.setState(AppState::Disconnected);
    }
}

void AuthController::slotDeleteFinished(bool success)
{
    LOG_INFO("Delete {}", success ? "OK" : "FAILED");
}
