#include "AccountDialog.h"
#include "ui_AccountDialog.h"
AccountDialog::AccountDialog(QWidget *parent): QMainWindow(parent), ui(new Ui::AccountDialog)
{
    ui->setupUi(this);
    connect(ui->authPushButton, &QPushButton::clicked, this, &AccountDialog::slotClickButton);
}

AccountDialog::~AccountDialog()
{
    delete ui;
}

void AccountDialog::slotClickButton()
{
    emit signalClickedButton(ui->usernameLineEdit->text(), ui->passwordLineEdit->text());
}
