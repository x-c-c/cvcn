#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Model.h"
#include "View.h"

class Controller: public QObject
{
    Q_OBJECT;
public:
    Controller(Model& model, View& view);
private:
    Model& model_;
    View& view_;

private slots:
    void slotOnConnectRequested(QString& username, QString& password);
};

#endif // CONTROLLER_H
