#include "View.h"
#include "ui_View.h"
View::View(QWidget *parent): QMainWindow(parent), ui(new Ui::View)
{
    ui->setupUi(this);
    connect(ui->sendPushButton, &QPushButton::clicked, this, &View::slotClickButton);
}

View::~View()
{
    delete ui;
}

void View::slotClickButton()
{
    emit signalClickedButton(ui->usernameLineEdit->text(), ui->passwordLineEdit->text());
}
