#include "MainController.h"
#include "Logger.h"

MainController::MainController(ProtocolClient& protocolClient,
                               AccountDialog& accountDialog,
                               ChatWindow& chatWindow):
    QObject(nullptr),
    protocolClient_(protocolClient),
    accountDialog_(accountDialog),
    chatWindow_(chatWindow),
    session_(),
    authController_(protocolClient, accountDialog, session_),
    chatController_(protocolClient, chatWindow),
    messageController_(protocolClient, chatWindow, session_)
{
    // AuthController -> MainController
    connect(&authController_, &AuthController::signalAuthenticationSucceeded,
            this, &MainController::slotAuthenticationSucceeded);
    connect(&authController_, &AuthController::signalShowWarning,
            this, &MainController::slotShowWarning);

    // ChatController -> MainController
    connect(&chatController_, &ChatController::signalShowInformation,
            this, &MainController::slotShowInformation);
    connect(&chatController_, &ChatController::signalShowWarning,
            this, &MainController::slotShowWarning);

    // MessageController -> MainController
    connect(&messageController_, &MessageController::signalShowWarning,
            this, &MainController::slotShowWarning);

    // ProtocolClient -> MainController
    connect(&protocolClient_, &ProtocolClient::signalErrorOccurred,
            this, &MainController::slotError);
    connect(&protocolClient_, &ProtocolClient::signalConnected,
            this, &MainController::slotConnected);
    connect(&protocolClient_, &ProtocolClient::signalDisconnected,
            this, &MainController::slotDisconnected);
}

void MainController::connectToServer(const QString& host, quint16 port)
{
    protocolClient_.connectToServer(host, port);
}

void MainController::slotAboutToQuit()
{
    LOG_INFO("Client shutting down, sending DisconnectRequest");
    protocolClient_.sendDisconnectRequest();
}

void MainController::slotAuthenticationSucceeded()
{
    LOG_INFO("Switching to chat window");
    accountDialog_.hide();
    chatWindow_.show();
    chatController_.loadChatList();
}

void MainController::slotError(ErrorKind kind, const QString& errorString)
{
    switch (kind)
    {
    case ErrorKind::Transport:
        LOG_ERROR("Transport error: {}", errorString.toStdString());
        session_.setState(AppState::Disconnected);
        break;
    case ErrorKind::Protocol:
        LOG_ERROR("Protocol error: {}", errorString.toStdString());
        break;
    case ErrorKind::Business:
        LOG_WARN("Business error: {}", errorString.toStdString());
        break;
    }
}

void MainController::slotShowInformation(const QString& title, const QString& text)
{
    chatWindow_.showInformation(title, text);
}

void MainController::slotShowWarning(const QString& title, const QString& text)
{
    chatWindow_.showWarning(title, text);
}

void MainController::slotConnected()
{
    LOG_INFO("Connected to server");
    session_.setState(AppState::Connected);
}

void MainController::slotDisconnected()
{
    LOG_WARN("Disconnected from server");
    session_.setState(AppState::Disconnected);
}
