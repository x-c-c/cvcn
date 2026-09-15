/**
 * @file    EventPoller.h
 * @brief   Обёртка над epoll: ждёт события на файловых дескрипторах
 *          и дёргает callbacks.
 *
 * @details
 *   Класс ничего не знает о протоколе, базе данных или клиентских
 *   сессиях. Его задача — принять fd, зарегистрировать его в epoll,
 *   крутить epoll_wait и в нужный момент вызвать нужный callback.
 *   Вся логика «что делать с событием» передаётся снаружи через
 *   std::function-поля.
 *
 *   Зарегистрировать можно один слушающий сокет и произвольное число
 *   клиентских. Каждый клиентский регистрируется в режиме
 *   EPOLLIN | EPOLLET — edge-triggered, что требует читать из сокета
 *   до EAGAIN в вызывающем коде (ClientSession::handleRead).
 * @see     ClientSession, SessionManager, ListeningSocket
 */

#pragma once

#include <sys/epoll.h>
#include <atomic>
#include <functional>
#include <stdexcept>
#include <string>

class EventPoller
{
public:
    EventPoller();
    ~EventPoller();

    // Типы колбэков 
    // std::function, а не указатель на функцию, чтобы можно было
    // передавать лямбды с захватом (см. main.cpp: [&sessions]).

    /// @brief Новое входящее соединение: fd уже неблокирующий и в epoll.
    using newConnectionCallback = std::function<void(int clientSocketFD)>;

    /// @brief На клиентском fd есть данные для чтения.
    using readEventCallback = std::function<void(int fileDescriptor)>;

    /// @brief На клиентском fd можно писать (сокет снова свободен).
    using writeEventCallback = std::function<void(int fileDescriptor)>;

    /// @brief Ошибка или закрытие (EPOLLERR | EPOLLHUP) на fd.
    using errorEventCallback = std::function<void(int fileDescriptor, uint32_t events)>;

    // Установка колбэков 
    // Можно вызывать до startEpollLoop.
    void setNewConnectionCallback(newConnectionCallback cb);
    void setReadEventCallback(readEventCallback cb);
    void setWriteEventCallback(writeEventCallback cb);
    void setErrorEventCallback(errorEventCallback cb);

    /**
     * @brief Запускает бесконечный epoll-цикл до остановки.
     *
     * @param serverSocketFD Слушающий сокет. Функция сама переведёт его
     *                       в неблокирующий режим и зарегистрирует
     *                       на EPOLLIN.
     *
     * @details
     *   Блокирующий вызов. Выходит, когда:
     *     - ShutdownSignal::isStopRequested() == true (Ctrl+C / SIGTERM);
     *     - на слушающем сокете EPOLLERR/EPOLLHUP;
     *     - epoll_wait вернул фатальную ошибку.
     *
     * @note    При EINTR (прерывание сигналом) цикл не завершается,
     *          а продолжается — это не ошибка.
     */
    void startEpollLoop(int serverSocketFD);

    /**
     * @brief Останавливает цикл и закрывает epoll-дескриптор.
     */
    void stopEpollLoop();

    /**
     * @brief Меняет набор событий, которые epoll отслеживает для fd.
     *
     * @details
     *   Используется PacketSender: при появлении данных в очереди
     *   добавляет EPOLLOUT, при опустошении — возвращает только EPOLLIN.
     *
     * @param fileDescriptor Клиентский fd, уже зарегистрированный.
     * @param events         Новый набор битов: EPOLLIN | EPOLLOUT и т.п.
     */
    void modifyFdEvents(int fileDescriptor, uint32_t events);

private:
    newConnectionCallback onNewConnection_;
    readEventCallback     onRead_;
    writeEventCallback    onWrite_;
    errorEventCallback    onError_;

    int epollFD_ = -1;                       ///< Владеет. Закрывается в stopEpollLoop().
    std::atomic<bool> running_{false};       ///< Флаг остановки цикла.

    // Внутренние операции с epoll 
    /// @brief Регистрирует fd в epoll с указанными событиями.
    /// @note  При ошибке только логирует: решение о фатальности
    ///        принимает вызывающий код (для серверного fd — фатально).
    void addFdToEpoll(int fileDescriptor, uint32_t events);

    /// @brief Снимает fd с учёта в epoll.
    void removeFdFromEpoll(int fileDescriptor);

    static constexpr int MAX_EVENTS = 1024;        ///< Размер буфера под события.
    static constexpr int WAIT_MILLISECONDS = 1000; ///< Таймаут epoll_wait, мс.
};
