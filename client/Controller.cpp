#include "Controller.h"

Controller::Controller(Model& model, View& view):
        QObject(nullptr), model_(model), view_(view)
{
    model_.connectToServer();
    connect(&view_, &View::signalClickedButton, this, &Controller::slotOnConnectRequested);
}

void Controller::slotOnConnectRequested(QString& username, QString& password)
{

}
