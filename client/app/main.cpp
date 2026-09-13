#include "Connection.h"
#include "ProtocolClient.h"
#include "AccountDialog.h"
#include "ChatWindow.h"
#include "MainController.h"
#include "Logger.h"
#include "AppConfig.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LOG_INFO("Client starting up");

    Connection connection;
    ProtocolClient protocolClient(&connection);
    AccountDialog accountDialog;
    ChatWindow chatWindow;
    MainController mainController(protocolClient, accountDialog, chatWindow);

    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &mainController, &MainController::slotAboutToQuit);

    accountDialog.show();
    mainController.connectToServer(
        QString::fromUtf8(config::DEFAULT_SERVER_HOST),
        config::DEFAULT_SERVER_PORT);

    return app.exec();
}
