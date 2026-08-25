#include "Model.h"
#include "View.h"
#include "Controller.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Model model;
    View view;
    Controller controller(model, view);

    view.show();
    return app.exec();
}
