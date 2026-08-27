#include "Controller.h"
#include <QDebug>
Controller::Controller(Model& model, AccountDialog& view):
        QObject(nullptr), model_(model), view_(view)
{
    model_.connectToServer("127.0.0.1", 55550);
    connect(&view_, &AccountDialog::signalClickedButton, this, &Controller::slotOnConnectRequested);
}

void Controller::slotOnConnectRequested(const QString& username, const QString& password)
{
    qDebug() << "Controller is GOOOD";
}
