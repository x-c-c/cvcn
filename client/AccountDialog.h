#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class AccountDialog;
}
QT_END_NAMESPACE

class AccountDialog : public QMainWindow
{
    Q_OBJECT

public:
    AccountDialog(QWidget *parent = nullptr);
    ~AccountDialog();
private:
    Ui::AccountDialog *ui;
    QString username_;
    QString password_;
private slots:
    void  slotClickedAuthButton();
    void  slotClickedRegButton();
    void  slotClickedDelButton();

signals:
    void signalAuthRequested(const QString& username, const QString& password);
    void signalRegRequested(const QString& username, const QString& password);
    void signalDelRequested(const QString& username, const QString& password);
};
#endif // ACCOUNTDIALOG_H
