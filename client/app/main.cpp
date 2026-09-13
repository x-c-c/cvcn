#include "ProtocolClient.h"
#include "AccountDialog.h"
#include "ChatWindow.h"
#include "MainController.h"
#include "Logger.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LOG_INFO("Client starting up");

    ProtocolClient protocolClient;
    AccountDialog accountDialog;
    ChatWindow chatWindow;
    MainController mainController(protocolClient, accountDialog, chatWindow);;

    accountDialog.show();
    mainController.connectToServer("127.0.0.1", 55550);

    return app.exec();
}
