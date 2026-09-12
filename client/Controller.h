#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include "Model.h"
#include "AccountDialog.h"

class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(Model& model, AccountDialog& view);

    void connectToServer(const QString& host, quint16 port);

private:
    Model& model_;
    AccountDialog& view_;

private slots:
    void slotAuthRequested(const QString& username, const QString& password);
    void slotRegRequested(const QString& username, const QString& password);
    void slotDelRequested(const QString& username, const QString& password);

    void onRegistrationFinished(bool success);
    void onAuthFinished(bool success, uint32_t sessionID);
    void onDeleteFinished(bool success);
    void onError(const QString& errorString);
};

#endif // CONTROLLER_H
