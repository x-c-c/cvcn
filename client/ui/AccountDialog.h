/**
 * @file    AccountDialog.h
 * @brief   Окно входа: логин, пароль и три кнопки — auth, register, delete.
 *
 * @details
 *   Окно не содержит бизнес-логики и не общается с сетью. Его задача —
 *   собрать введённые пользователем строки и сообщить наружу, какое
 *   действие он выбрал. Связь с остальным миром — через сигналы,
 *   которые ловит Controller.
 *
 *   Сигналы:
 *     - signalAuthRequested — нажата кнопка «authenticate»;
 *     - signalRegRequested  — нажата кнопка «register»;
 *     - signalDelRequested  — нажата кнопка «delete».
 *
 *   Все три сигнала несут одну и ту же пару (username, password).
 *   Различаются они намерением: войти, зарегистрироваться или удалить
 *   аккаунт. Controller решает, что с этим делать.
 *
 * @note    Реализовано через QMainWindow, потом нужно заменить
 * @warning Никакой валидации здесь нет. Если пользователь оставит
 *          поле пустым, сигнал всё равно уйдёт — с пустой строкой.
 *          Проверять — на стороне Controller/AuthController.
 * @see     Controller, Model, AccountDialog.ui
 */

#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QMainWindow>

// Forward declaration Ui::AccountDialog. Полный тип нужен только
// в .cpp, где подключается сгенерированный ui_AccountDialog.h.
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
    /**
     * @brief Создаёт окно и подключает слоты к кнопкам.
     * @param parent Родитель в дереве Qt.
     */
    explicit AccountDialog(QWidget *parent = nullptr);

    /**
     * @brief Удаляет сгенерированный Ui::AccountDialog.
     */
    ~AccountDialog();

private:
    Ui::AccountDialog *ui;      ///< Владеет. Удаляется в деструкторе.
    QString username_;          ///< Последнее введённое имя пользователя.
    QString password_;          ///< Последний введённый пароль.

private slots:
    /**
     * @brief Реакция на кнопку «authenticate».
     *
     * @details
     *   Читает текст из полей usernameLineEdit и passwordLineEdit
     *   в username_/password_, затем эмитит signalAuthRequested.
     */
    void slotClickedAuthButton();

    /**
     * @brief Реакция на кнопку «register».
     * @details Заполняет username_/password_ и эмитит signalRegRequested.
     */
    void slotClickedRegButton();

    /**
     * @brief Реакция на кнопку «delete».
     * @details Заполняет username_/password_ и эмитит signalDelRequested.
     */
    void slotClickedDelButton();

signals:
    /**
     * @brief Пользователь запросил вход.
     * @param username Имя из поля ввода.
     * @param password Пароль из поля ввода.
     */
    void signalAuthRequested(const QString& username, const QString& password);

    /**
     * @brief Пользователь запросил регистрацию.
     * @param username Имя из поля ввода.
     * @param password Пароль из поля ввода.
     */
    void signalRegRequested(const QString& username, const QString& password);

    /**
     * @brief Пользователь запросил удаление аккаунта.
     * @param username Имя из поля ввода.
     * @param password Пароль из поля ввода.
     */
    void signalDelRequested(const QString& username, const QString& password);
};
#endif // ACCOUNTDIALOG_H
