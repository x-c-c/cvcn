/**
 * @file    SessionManager.h
 * @brief   Владеет сессиями клиентов и связывает их с EventPoller и Database.
 *
 * @details
 *   Хранит по одному ClientSession* на fd в unordered_map. Владеет
 *   сессиями: создаёт через new в onNewConnection(), удаляет через
 *   delete при закрытии или в деструкторе. EventPoller-ом и
 *   Database-ом не владеет — получает их ссылками, живут дольше.
 *
 *   Роль Manager по формальной модели: управляет жизненным циклом
 *   коллекции объектов. Сессии не удаляют себя сами — только
 *   помечают closed_, а решение об delete принимает менеджер
 *   (иначе в карте остался бы висячий указатель).
 *
 *   Жизненный цикл сессии:
 *     onNewConnection(fd) → new ClientSession → sessions_[fd]
 *     onRead/onWrite(fd)  → handle* → eraseIfClosed(fd)
 *     onError(fd)         → destroySession(fd)
 *     ~SessionManager     → closeAll() для всех оставшихся.
 * @see     ClientSession, EventPoller, Database
 */

#pragma once

#include <unordered_map>
#include <cstdint>

class ClientSession;
class EventPoller;
class Database;

class SessionManager
{
public:
    /**
     * @brief Конструирует менеджер, привязанный к поллеру и БД.
     *
     * @param epoller EventPoller, который будет получать события на
     *                клиентских fd. Не владеет.
     * @param db      Общая база данных сервера. Не владеет.
     */
    SessionManager(EventPoller& epoller, Database& db);

    /**
     * @brief Деструктор. Удаляет все оставшиеся сессии через closeAll().
     */
    ~SessionManager();

    SessionManager(const SessionManager&) = delete;             ///< Владеет сессиями.
    SessionManager& operator=(const SessionManager&) = delete;

    /**
     * @brief Создаёт ClientSession для нового подключения.
     *
     * @param clientSocketFD Клиентский fd, уже неблокирующий и
     *                       зарегистрированный в epoll.
     */
    void onNewConnection(int clientSocketFD);

    /**
     * @brief Обрабатывает событие EPOLLIN на клиентском fd.
     *
     * @param fd Клиентский дескриптор.
     *
     * @details
     *   Находит сессию по fd, вызывает handleRead(), затем —
     *   eraseIfClosed(fd), чтобы убрать мёртвую сессию из карты.
     *   Если сессии по fd нет — молча выходит (событие могло прийти
     *   уже после закрытия).
     */
    void onRead(int fd);

    /**
     * @brief Обрабатывает событие EPOLLOUT на клиентском fd.
     * @param fd Клиентский дескриптор.
     * @details
     *   Аналогично onRead: handleWrite() + eraseIfClosed(fd).
     */
    void onWrite(int fd);

    /**
     * @brief Обрабатывает EPOLLERR / EPOLLHUP на клиентском fd.
     *
     * @param fd     Клиентский дескриптор.
     * @param events Битовая маска событий — для диагностики.
     */
    void onError(int fd, uint32_t events);

    /**
     * @brief Удаляет все сессии. Вызывается в деструкторе.
     *
     * @details
     *   Для каждой сессии: закрыть fd (если ещё открыт), delete,
     *   очистить карту. После вызова sessions_ пуста.
     */
    void closeAll();

private:
    EventPoller& epoller_;   ///< Не владеет. Живёт дольше менеджера.
    Database& db_;           ///< Не владеет. Общая на весь сервер.
    std::unordered_map<int, ClientSession*> sessions_;  ///< Владеет значениями.

    /**
     * @brief Удаляет сессию по fd, если её closed_ == true.
     * @param fd Клиентский дескриптор.
     * @details
     *   Вызывается после handleRead/handleWrite: обработчик мог
     *   закрыть сессию (EOF, DisconnectRequest, ошибка). Если карта
     *   больше не должна на неё смотреть — delete + erase.
     *   Если сессия жива — ничего не делает.
     */
    void eraseIfClosed(int fd);

    /**
     * @brief Принудительно удаляет сессию по fd, закрывая её.
     * @param fd Клиентский дескриптор.
     * @details
     *   Используется в onNewConnection (страховка) и onError
     *   (безусловное удаление). Если сессии нет — просто выходит.
     *   Если есть — сначала closeSession, потом delete + erase.
     */
    void destroySession(int fd);
};
