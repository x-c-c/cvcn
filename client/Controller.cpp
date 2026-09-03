#include "Controller.h"
#include <QDebug>
Controller::Controller(Model& model, AccountDialog& view): QObject(nullptr), model_(model), view_(view)
{
    model_.connectToServer("127.0.0.1", 55550);
    connect(&view_, &AccountDialog::signalAuthRequested, this, &Controller::slotAuthRequested);
    connect(&view_, &AccountDialog::signalRegRequested, this, &Controller::);
    connect(&view_, &AccountDialog::signalDelRequested, this, &Controller::);
}

void Controller::slotRegRequested(const QString& username, const QString& password)
{
    model_.sendRegRequest(username, password);
}
