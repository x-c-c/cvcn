#include "Controller.h"
#include <QDebug>
#include "Validator.h"

Controller::Controller(Model& model, AccountDialog& view): QObject(nullptr), model_(model), view_(view)
{
    connect(&view_, &AccountDialog::signalAuthRequested, this, &Controller::slotAuthRequested);
    connect(&view_, &AccountDialog::signalRegRequested,  this, &Controller::slotRegRequested);
    connect(&view_, &AccountDialog::signalDelRequested,  this, &Controller::slotDelRequested);

    connect(&model_, &Model::registrationFinished, this, &Controller::onRegistrationFinished);
    connect(&model_, &Model::authFinished,         this, &Controller::onAuthFinished);
    connect(&model_, &Model::deleteFinished,       this, &Controller::onDeleteFinished);
    connect(&model_, &Model::errorOccurred,        this, &Controller::onError);
}

void Controller::connectToServer(const QString& host, quint16 port)
{
    model_.connectToServer(host, port);
}

void Controller::slotAuthRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        qWarning() << "Auth rejected locally: invalid username or password format";
        return;
    }
    model_.sendAuthRequest(username, password);
}

void Controller::slotRegRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        qWarning() << "Register rejected locally: invalid username or password format";
        return;
    }
    model_.sendRegRequest(username, password);
}

void Controller::slotDelRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        qWarning() << "Delete rejected locally: invalid username or password format";
        return;
    }
    model_.sendDeleteRequest(username, password);
}

void Controller::onRegistrationFinished(bool success)
{
    qDebug() << "Registration" << (success ? "OK" : "FAILED");
}

void Controller::onAuthFinished(bool success, uint32_t sessionID)
{
    qDebug() << "Auth" << (success ? "OK" : "FAILED") << "sessionID =" << sessionID;
}

void Controller::onDeleteFinished(bool success)
{
    qDebug() << "Delete" << (success ? "OK" : "FAILED");
}

void Controller::onError(const QString& errorString)
{
    qDebug() << "Error:" << errorString;
}
