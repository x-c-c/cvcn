#include "Connection.h"
#include "ProtocolClient.h"
#include "AccountDialog.h"
#include "ChatWindow.h"
#include "MainController.h"
#include "Logger.h"
#include "AppConfig.h"
#include <QApplication>
#include <QMessageBox>
#include <exception>
#include <cstdlib>
#include <iostream>

namespace {

void terminateHandler()
{
    std::cerr << "[fatal] Unhandled exception, terminating" << std::endl;
    std::abort();
}

} // namespace

int main(int argc, char *argv[])
{
    std::set_terminate(terminateHandler);

    try
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
    catch (const std::exception& e)
    {
        std::cerr << "[fatal] " << e.what() << std::endl;
        LOG_CRITICAL("Fatal exception: {}", e.what());
        QMessageBox::critical(nullptr,
                              QStringLiteral("Fatal error"),
                              QString::fromUtf8(e.what()));
        return 1;
    }
    catch (...)
    {
        std::cerr << "[fatal] Unknown exception" << std::endl;
        LOG_CRITICAL("Fatal unknown exception");
        QMessageBox::critical(nullptr,
                              QStringLiteral("Fatal error"),
                              QStringLiteral("Unknown exception"));
        return 1;
    }
}
