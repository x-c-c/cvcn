#include "./Model.h"
#include "../ui/AccountDialog.h"
#include "./Controller.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Model model;
    AccountDialog view;
    Controller controller(model, view);

    view.show();
    return app.exec();
}
