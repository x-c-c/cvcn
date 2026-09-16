/**
 * @file    AccountDialog.cpp
 * @brief   Реализация AccountDialog.
 *
 * @details
 *   Слоты-обработчики кнопок читают строки из QLineEdit и эмитят
 *   соответствующий сигнал. Никакой логики «что делать дальше» —
 *   это ответственность Controller.
 *
 * @see AccountDialog.h
 */

#include "./AccountDialog.h"
#include "ui_AccountDialog.h"

AccountDialog::AccountDialog(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::AccountDialog)
{
    ui->setupUi(this);

    // Связываем каждую кнопку с соответствующим слотом.
    connect(ui->authPushButton, &QPushButton::clicked,
            this, &AccountDialog::slotClickedAuthButton);
    connect(ui->regPushButton, &QPushButton::clicked,
            this, &AccountDialog::slotClickedRegButton);
    connect(ui->delPushButton, &QPushButton::clicked,
            this, &AccountDialog::slotClickedDelButton);
}

AccountDialog::~AccountDialog()
{
    delete ui;
}

void AccountDialog::slotClickedAuthButton()
{
    // Сохраняем введённые строки в поля класса — на случай, если
    // контроллер захочет их перечитать (например, при повторной
    // попытке входа после ошибки).
    username_ = ui->usernameLineEdit->text();
    password_ = ui->passwordLineEdit->text();

    // Сигнал уходит в Controller, тот решает, что с ним делать.
    emit signalAuthRequested(username_, password_);
}

void AccountDialog::slotClickedRegButton()
{
    username_ = ui->usernameLineEdit->text();
    password_ = ui->passwordLineEdit->text();
    emit signalRegRequested(username_, password_);
}

void AccountDialog::slotClickedDelButton()
{
    username_ = ui->usernameLineEdit->text();
    password_ = ui->passwordLineEdit->text();
    emit signalDelRequested(username_, password_);
}
