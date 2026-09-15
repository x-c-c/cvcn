/**
 * @file    PacketSender.cpp
 * @brief   Реализация PacketSender.
 *
 * @details
 *   Ключевой момент — работа с неблокирующим сокетом. send() не
 *   гарантирует, что примет весь буфер за один вызов. Поэтому
 *   flushSendQueue() может выйти с частично отправленным пакетом
 *   (остаток хранится первым в очереди) и включает EPOLLOUT,
 *   чтобы EventPoller разбудил сессию, когда сокет снова будет готов.
 *
 * @see PacketSender.h
 */

#include "./PacketSender.h"
#include "../net/EventPoller.h"
#include "../utils/Logger.h"
#include <cerrno>
#include <unistd.h>

PacketSender::PacketSender(EventPoller* epoller, int socketDescriptor):
    epoller_(epoller),
    socketDescriptor_(socketDescriptor)
{
}

void PacketSender::armWriteNotification()
{
    // Добавляем EPOLLOUT: как только ядро освободит буфер сокета,
    // EventPoller вызовет handleWrite() → flushSendQueue().
    // EPOLLIN остаётся — мы всё ещё хотим читать от клиента.
    epoller_->modifyFdEvents(socketDescriptor_, EPOLLIN | EPOLLOUT);

    // Флаг нужен, чтобы sendResponse() знал: не пытайся отправлять
    // сразу, это уже делает текущая незавершённая отправка.
    writePending_ = true;
}

void PacketSender::flushSendQueue()
{
    while (!sendQueue_.empty())
    {
        std::vector<uint8_t>& buffer = sendQueue_.front();

        // MSG_NOSIGNAL: не получать SIGPIPE при разрыве соединения.
        // EPOLLET/неблокирующий сокет: при переполнении вернёт EAGAIN.
        const ssize_t sent = send(socketDescriptor_, buffer.data(),
                                  buffer.size(), MSG_NOSIGNAL);

        if (sent < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Сокет забит: ждём EPOLLOUT, остаток остаётся в очереди.
                armWriteNotification();
                return;
            }

            // Реальная ошибка (EPIPE, ECONNRESET и т.п.). Логируем
            // и выходим — закрывать сессию будет вызывающий код
            // (ClientSession::closeSession через onError/eraseIfClosed).
            Logger::instance().error("send error on fd {}: {}",
                                     socketDescriptor_, strerror(errno));
            return;
        }

        if (static_cast<size_t>(sent) < buffer.size())
        {
            // Частичная отправка: убираем отправленный префикс,
            // остаток сохраняем первым в очереди и ждём EPOLLOUT.
            buffer.erase(buffer.begin(), buffer.begin() + sent);
            armWriteNotification();
            return;
        }

        // Пакет ушёл целиком — переходим к следующему.
        sendQueue_.pop_front();
    }

    // Очередь пуста: снимаем EPOLLOUT, оставляем только EPOLLIN.
    // Это важно: иначе epoll будет постоянно будить нас на EPOLLOUT,
    // пока мы не отправим хоть что-то, хотя отправлять нечего.
    epoller_->modifyFdEvents(socketDescriptor_, EPOLLIN);
    writePending_ = false;
}

void PacketSender::sendResponse(const std::vector<uint8_t>& data)
{
    // Всегда добавляем в очередь: если прямо сейчас идёт отправка,
    // новый пакет должен уйти после остатка предыдущего, а не
    // вклиниться в середину.
    sendQueue_.push_back(data);

    // Если отправка не «висит» (сокет свободен) — пробуем отправить
    // немедленно. Если «висит» — следующий EPOLLOUT сам всё сделает.
    if (!writePending_)
        flushSendQueue();
}

void PacketSender::handleWrite()
{
    // EventPoller дёрнул нас по EPOLLOUT: сокет готов принять ещё.
    // Сбрасываем флаг перед flush — иначе sendResponse() от нового
    // пакета решит, что запись всё ещё идёт, и просто положит его
    // в очередь, не попробовав отправить.
    writePending_ = false;
    flushSendQueue();
}
