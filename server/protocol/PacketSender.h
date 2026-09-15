/**
 * @file    PacketSender.h
 * @brief   Очередь исходящих пакетов для одного клиентского сокета.
 *
 * @details
 *   Отвечает за надёжную отправку: сокет неблокирующий, send() может
 *   принять только часть буфера. Класс копит пакеты в очереди
 *   std::deque и отправляет столько, сколько сокет принимает прямо
 *   сейчас. Остаток ждёт события EPOLLOUT — EventPoller разбудит
 *   сессию, и handleWrite() продолжит дренаж.
 *
 *   Жизненный цикл отправки одного пакета:
 *     1. sendResponse() кладёт data в очередь.
 *     2. Если сейчас ничего не «висит» — сразу пробует flushSendQueue().
 *     3. Если сокет принял всё — пакет уходит.
 *     4. Если частично (EAGAIN) — остаток остаётся в очереди,
 *        включается EPOLLOUT через armWriteNotification().
 *     5. На EPOLLOUT EventPoller вызовет handleWrite() — продолжение
 *        с того места, где остановились.
 *
 *   PacketSender не владеет ни сокетом, ни epoll-ом — только
 *   указатель/дескриптор. Удаляет его ClientSession вместе с собой.
 *
 * @see     ClientSession, EventPoller
 */

#pragma once

#include <vector>
#include <cstdint>
#include <deque>
#include <sys/socket.h>

class EventPoller;

class PacketSender
{
public:
    /**
     * @brief Конструирует отправитель для конкретного сокета.
     *
     * @param epoller          Поллер, который будет разбужен по EPOLLOUT,
     *                         когда в очереди останется непосланное.
     *                         Не владеет.
     * @param socketDescriptor Клиентский fd. Не владеет, не закрывает.
     */
    PacketSender(EventPoller* epoller, int socketDescriptor);

    /**
     * @brief Ставит пакет в очередь и, если можно, сразу отправляет.
     *
     * @param data Готовый к отправке пакет (уже с заголовком и телом).
     *
     * @details
     *   Всегда кладёт data в конец очереди. Если прямо сейчас нет
     *   незавершённой отправки (writePending_ == false) — вызывает
     *   flushSendQueue(), чтобы попробовать отправить немедленно.
     *   Если запись уже «висит» (сокет забит), новый пакет просто
     *   встанет в очередь и уйдёт вместе со следующим flush.
     */
    void sendResponse(const std::vector<uint8_t>& data);

    /**
     * @brief Продолжает отправку после события EPOLLOUT. Вызывается
     *        EventPoller-ом через ClientSession::handleWrite().
     *
     * @details
     *   Сбрасывает флаг writePending_ и запускает flushSendQueue()
     *   заново с начала очереди. Первый буфер в очереди — тот,
     *   что остался недосланным в прошлый раз (flush хранит
     *   частично отправленный буфер первым элементом).
     */
    void handleWrite();

private:
    EventPoller* epoller_;              ///< Не владеет. Живёт дольше.
    int socketDescriptor_;              ///< Не владеет. Закрывает ClientSession.
    std::deque<std::vector<uint8_t>> sendQueue_;  ///< Владеет. FIFO.
    bool writePending_ = false;         ///< true — ждём EPOLLOUT.

    /**
     * @brief Отправляет столько, сколько сокет примет. При частичной
     *        отправке — оставляет остаток и включает EPOLLOUT.
     *
     * @details
     *   Цикл: берём front(), send(). Возможные исходы:
     *     - sent == buffer.size() — пакет ушёл, pop_front() и дальше;
     *     - 0 < sent < size         — остаток сдвигается в начало буфера,
     *                                  armWriteNotification(), выход;
     *     - sent == -1, EAGAIN      — сокет забит, armWriteNotification();
     *     - sent == -1, другая errno — реальная ошибка, лог и выход;
     *     - sent == 0               — редкий случай, тоже выход.
     *
     *   Если очередь опустела — снимаем EPOLLOUT.
     */
    void flushSendQueue();

    /**
     * @brief Добавляет EPOLLOUT к событиям сокета в epoll.
     *
     * @details
     *   Вызывается, когда в очереди остались непосланные данные.
     *   Устанавливает writePending_ = true, чтобы sendResponse()
     *   знал: новая запись должна встать в очередь, а не пытаться
     *   отправиться немедленно.
     */
    void armWriteNotification();
};
