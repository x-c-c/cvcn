/**
 * @file    Controller.h
 * @brief   Координатор между UI и моделью.
 *
 * @details
 *
 *   Классический Controller из MVC: принимает намерения пользователя
 *   от View (AccountDialog) и превращает их в вызовы Model
 *   (sendAuthRequest / sendRegRequest / sendDelRequest). View и Model
 *   друг о друге не знают — вся связь проходит через этот класс.
 *
 *   Controller не владеет Model и AccountDialog: получает их ссылками.
 *   В конструкторе подключается к сигналам View и
 *   устанавливает TCP-соединение через Model.
 *
 * @warning Порт и адрес сервера захардкожены в конструкторе
 *          ("127.0.0.1", 55550). Для тестов и смены окружения стоит
 *          вынести в конфиг или аргументы командной строки.
 * @see     Model, AccountDialog
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include "./Model.h"
#include "../ui/AccountDialog.h"

class Controller : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Создаёт контроллер, связывает View и Model, подключается к серверу.
     *
     * @param model Сетевая модель клиента. Не владеет.
     * @param view  Окно входа. Не владеет.
     *
     * @details
     *   В конструкторе:
     *     1. Вызывается model.connectToServer("127.0.0.1", 55550) —
     *        TCP-соединение устанавливается сразу, до первого действия
     *        пользователя.
     *     2. Подключаются три сигнала AccountDialog к слотам
     *        Controller-а. С этого момента нажатия кнопок в окне
     *        входа будут доходить сюда.
     *
     */
    Controller(Model& model, AccountDialog& view);

private:
    Model& model_;              ///< Не владеет. Живёт дольше контроллера.
    AccountDialog& view_;       ///< Не владеет. Живёт дольше контроллера.

private slots:
    /**
     * @brief Пользователь нажал «authenticate».
     *
     * @param username Имя из поля ввода.
     * @param password Пароль из поля ввода.
     *
     * @details Просто переадресует запрос в Model::sendAuthRequest.
     *          Проверки (непустые поля, допустимые символы) —
     *          на стороне View или сервера.
     */
    void slotAuthRequested(const QString& username, const QString& password);

    /**
     * @brief Пользователь нажал «register».
     *
     * @param username Имя из поля ввода.
     * @param password Пароль из поля ввода.
     *
     * @details Переадресует в Model::sendRegRequest.
     */
    void slotRegRequested(const QString& username, const QString& password);

    /**
     * @brief Пользователь нажал «delete».
     *
     * @param username Имя из поля ввода.
     * @param password Пароль из поля ввода.
     *
     * @warning Слот пока пуст: Model::sendDeleteRequest не реализован,
     *          delete-запрос не уходит на сервер. Нажатие кнопки
     *          «delete» в UI ничего не делает.
     */
    void slotDelRequested(const QString& username, const QString& password);
};

#endif // CONTROLLER_H
