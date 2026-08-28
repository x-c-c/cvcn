#include "AccountDialog.h"
#include "ui_AccountDialog.h"
AccountDialog::AccountDialog(QWidget *parent): QMainWindow(parent), ui(new Ui::AccountDialog)
{
    ui->setupUi(this);
    connect(ui->authPushButton, &QPushButton::clicked, this, &AccountDialog::slotClickedAuthButton);
    connect(ui->regPushButton, &QPushButton::clicked, this, &AccountDialog::slotClickedRegButton);
    connect(ui->delPushButton, &QPushButton::clicked, this, &AccountDialog::slotClickedDelButton);
}

AccountDialog::~AccountDialog()
{
    delete ui;
}

void AccountDialog::slotClickedAuthButton()
{
    username_ =  ui->usernameLineEdit->text();
    password_ = ui->passwordLineEdit->text();
    emit signalAuthRequested(username_, password_);
}
void AccountDialog::slotClickedRegButton()
{
    username_ =  ui->usernameLineEdit->text();
    password_ = ui->passwordLineEdit->text();
    emit signalRegRequested(username_, password_);
}
void AccountDialog::slotClickedDelButton()
{
    username_ =  ui->usernameLineEdit->text();
    password_ = ui->passwordLineEdit->text();
    emit signalDelRequested(username_, password_);
}
