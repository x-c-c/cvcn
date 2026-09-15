#pragma once
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

class ClientSession;
class PacketDispatcher;
class SessionRegistry;
class EventPoller;
class ThreadPool;
class ResultQueue;

/**
 * @brief Управляет жизненным циклом клиентских сессий.
 *
 * Хранит отображение fd → ClientSession*, счётчик in-flight задач
 * на каждой сессии и множество fd, ожидающих отложенного удаления.
 *
 * @par Роль в архитектуре:
 *   Связывает EventPoller, ThreadPool и ResultQueue: main-thread
 *   вызывает onRead/onWrite/onError, воркеры не трогают SessionManager
 *   напрямую, а возвращают результат через ResultQueue.
 *
 * @par Жизненный цикл сессии:
 *   1. onNewConnection(fd) — создаётся ClientSession.
 *   2. onRead(fd) — пакеты извлекаются, задачи уходят в ThreadPool.
 *   3. На время работы задачи воркера счётчик inFlight_[fd] > 0.
 *   4. Когда сессия должна быть закрыта, но счётчик > 0, fd попадает
 *      в pendingDelete_. closeClient() откладывается до onTaskDone().
 *   5. Закрытие: unregisterUser из SessionRegistry, closeSession,
 *      delete, erase из всех карт.
 *
 * @note Все методы вызываются только из main-thread.
 * @see ClientSession, EventPoller, ThreadPool, ResultQueue
 */

class SessionManager
{
public:
    SessionManager(PacketDispatcher* dispatcher,
                   SessionRegistry* sessionRegistry,
                   EventPoller* eventPoller,
                   ThreadPool* threadPool,
                   ResultQueue* resultQueue);
    ~SessionManager();

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    /**
     * @brief Создать сессию для нового клиента.
     *
     * @param fileDescriptor fd принятого сокета, уже в non-blocking
     *                       режиме и зарегистрированный в epoll.
     *
     * @post Сессия в sessions_[fd], счётчик inFlight_[fd] == 0.
     */

    void onNewConnection(int fileDescriptor);
    /**
     * @brief Обработать EPOLLIN на клиентском fd.
     *
     * @param fileDescriptor fd, готовый к чтению
     *
     * @post Все целые пакеты из сокета превратились в задачи ThreadPool;
     *       inFlight_[fd] увеличен на число задач.
     * @note main-thread.
     */

    void onRead(int fileDescriptor);
    /**
     * @brief Обработать EPOLLOUT на клиентском fd.
     * @param fileDescriptor fd, готовый к записи
     * @note main-thread.
     */

    void onWrite(int fileDescriptor);
    /**
     * @brief Обработать EPOLLERR/EPOLLHUP на клиентском fd.
     *
     * @param fileDescriptor fd с ошибкой
     * @param events         маска событий, пришедшая из epoll
     *
     * @post Сессия либо удалена, либо помечена как pendingDelete,
     *       если у неё ещё есть in-flight задачи.
     */

    void onError(int fileDescriptor, uint32_t events);

    /** @brief Обработать команду Close из ResultQueue. main-thread. */
    void requestCloseClient(int fileDescriptor);

    /** @brief Уменьшить in-flight по fd. main-thread. */
    void onTaskDone(int fileDescriptor);

    /**
     * @brief Найти сессию по fd.
     * @param fileDescriptor файловый дескриптор
     * @return указатель на сессию или nullptr.
     * @note Используется main-thread'ом при обработке SendRaw из ResultQueue.
     */

    ClientSession* getSession(int fileDescriptor);

private:
    PacketDispatcher* dispatcher_;
    SessionRegistry* sessionRegistry_;
    EventPoller* eventPoller_;
    ThreadPool* threadPool_;
    ResultQueue* resultQueue_;
    std::unordered_map<int, ClientSession*> sessions_;
    std::unordered_map<int, int> inFlight_;
    std::unordered_set<int> pendingDelete_;

    /**
     * @brief Немедленно удалить сессию.
     *
     * @param fileDescriptor fd удаляемой сессии
     *
     * @pre  inFlight_[fileDescriptor] == 0 (иначе UAF в воркере).
     * @post Сессия снята с epoll, закрыта, удалена из SessionRegistry
     *       и из всех карт SessionManager.
     */

    void closeClient(int fileDescriptor);
    /**
     * @brief Закрыть сессию или отложить удаление при in-flight.
     *
     * @param fileDescriptor fd закрываемой сессии
     *
     * @post Если inFlight_[fd] > 0 — fd добавлен в pendingDelete_,
     *       сессия снята с epoll, но объект жив.
     *       Иначе — вызов closeClient().
     * @note Используется из onRead/onError, где закрытие не связано
     *       с явным запросом воркера.
     */

    void closeOrDefer(int fileDescriptor);
};
