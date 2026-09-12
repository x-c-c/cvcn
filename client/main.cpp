#include "Model.h"
#include "AccountDialog.h"
#include "Controller.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Model model;
    AccountDialog view;
    Controller controller(model, view);

    view.show();
    controller.connectToServer("127.0.0.1", 55550);

    return app.exec();
}
