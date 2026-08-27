#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include "Model.h"
#include "AccountDialog.h"

class Controller: public QObject
{
    Q_OBJECT
public:
    Controller(Model& model, AccountDialog& view);
private:
    Model& model_;
    AccountDialog& view_;

private slots:
    void slotOnConnectRequested(const QString& username, const QString& password);
};

#endif // CONTROLLER_H
