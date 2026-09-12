#include "Model.h"
#include "AccountDialog.h"
#include "ChatWindow.h"
#include "Controller.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Model model;
    AccountDialog accountDialog;
    ChatWindow chatWindow;
    Controller controller(model, accountDialog, chatWindow);;

    accountDialog.show();
    controller.connectToServer("127.0.0.1", 55550);

    return app.exec();
}
