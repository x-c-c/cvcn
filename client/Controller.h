#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include "Model.h"
#include "View.h"

class Controller: public QObject
{
    Q_OBJECT
public:
    Controller(Model& model, View& view);
private:
    Model& model_;
    View& view_;

private slots:
    void slotOnConnectRequested(const QString& username, const QString& password);
};

#endif // CONTROLLER_H
