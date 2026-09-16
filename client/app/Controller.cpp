/**
 * @file    Controller.cpp
 * @brief   Реализация Controller.
 *
 * @details
 *   Конструктор связывает сигналы AccountDialog со слотами
 *   контроллера и сразу открывает TCP-соединение. Каждый слот —
 *   тонкая обёртка над соответствующим методом Model: никакой
 *   логики, только маршрутизация.
 *
 *   Схема потока управления:
 *     кнопка в UI → signalAuthRequested → slotAuthRequested
 *     → model.sendAuthRequest → socket.write(...)
  * @see Controller.h
 */

#include "./Controller.h"
#include <QDebug>

Controller::Controller(Model& model, AccountDialog& view):
    QObject(nullptr),
    model_(model),
    view_(view)
{
    // Подключение к серверу выполняется в конструкторе, до показа окна.
    model_.connectToServer("127.0.0.1", 55550);

    // Связываем сигналы View со слотами Controller-а.
    connect(&view_, &AccountDialog::signalAuthRequested,
            this, &Controller::slotAuthRequested);
    connect(&view_, &AccountDialog::signalRegRequested,
            this, &Controller::slotRegRequested);
    connect(&view_, &AccountDialog::signalDelRequested,
            this, &Controller::slotDelRequested);
}

void Controller::slotAuthRequested(const QString& username, const QString& password)
{
    // Никакой валидации здесь нет. Если поля пустые — на сервер
    // уйдёт AuthRequest с пустыми строками, сервер вернёт
    // AuthResponse с success = 0.
    model_.sendAuthRequest(username, password);
}

void Controller::slotRegRequested(const QString& username, const QString& password)
{
    // Аналогично slotAuthRequested: маршрутизация без логики.
    model_.sendRegRequest(username, password);
}

void Controller::slotDelRequested(const QString& username, const QString& password)
{
    // TODO: реализовать, когда появится Model::sendDeleteRequest
    // и DeleteRequestData в протоколе. Сейчас кнопка «delete»
    // в UI не работает: сигнал приходит, но ничего не делает.
    //
    // Параметры (void)username, (void)password нужны только чтобы
    // компилятор не ругался на неиспользуемые аргументы, если
    // раскомментировать тело.
    (void)username;
    (void)password;
}
