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
private slots:
    void  slotClickButton();
signals:
       void signalClickedButton(const QString& username, const QString& password );

};
#endif // ACCOUNTDIALOG_H
