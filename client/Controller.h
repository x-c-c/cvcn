#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include "Model.h"
#include "AccountDialog.h"
#include "ChatWindow.h"
class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(Model& model, AccountDialog& view, ChatWindow& chatWindow);
    void connectToServer(const QString& host, quint16 port);

private:
    Model& model_;
    AccountDialog& view_;
    ChatWindow& chatWindow_;
    QString currentUsername_;

private slots:
    void slotAuthRequested(const QString& username, const QString& password);
    void slotRegRequested(const QString& username, const QString& password);
    void slotDelRequested(const QString& username, const QString& password);

    void slotMessageSendRequested(const QString& text);

    void onRegistrationFinished(bool success);
    void onAuthFinished(bool success, uint32_t sessionID);
    void onDeleteFinished(bool success);
    void onError(const QString& errorString);
};

#endif // CONTROLLER_H
