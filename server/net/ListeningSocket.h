/**
 * @file    ListeningSocket.h
 * @brief   Владеет слушающим сокетом: создаёт, bind, listen, закрывает.
 *
 * @details
 *   Один экземпляр = один слушающий сокет. Класс не занимается
 *   accept-ом и не обрабатывает клиентов — только готовит «входную
 *   дверь» сервера. Как только сокет в состоянии listen, его
 *   дескриптор передаётся в EventPoller.
 *
 * @note    Класс не копируется: дескриптор уникален.
 * @see     ServerConfig, EventPoller
 */

#pragma once

#include "../config/ServerConfig.h"
#include <sys/socket.h>
#include <unistd.h>

class ListeningSocket
{
private:
    int serverSocketFD_ = -1;           ///< Владеет. -1 — сокет не создан/закрыт.
    sockaddr_in serverAddr{};           ///< Адрес, на котором слушаем (заполняется в initServerAddr).
    static constexpr int reuseAddrOption = 1;   ///< Значение для SO_REUSEADDR.

    /**
     * @brief Заполняет sockaddr_in из конфига.
     * @param config Настройки: domain, addr, port.
     * @note Порт переводится в network byte order через htons.
     *       Адрес уже в network order (INADDR_ANY или in_addr_t).
     */
    void initServerAddr(const ServerConfig& config);

public:
    /**
     * @brief Деструктор. Закрывает сокет, если он ещё открыт.
     */
    ~ListeningSocket();

    /// @brief Возвращает дескриптор слушающего сокета или -1, если не открыт.
    int getServerSocketFD() { return serverSocketFD_; }

    /**
     * @brief Создаёт сокет, привязывает к адресу и переводит в listen.
     * @param config Параметры сокета: domain, type, protocol, addr, port.
     * @throws std::runtime_error При ошибке socket(), bind(), listen().
     *         Текст содержит errno. После throw сокет закрыт,
     *         объект остаётся в валидном состоянии (serverSocketFD_ == -1).
     *
     * @details
     *   Шаги:
     *     1. Если сокет уже открыт — закрыть (переоткрытие).
     *     2. socket() с параметрами из конфига.
     *     3. setsockopt(SO_REUSEADDR) — чтобы bind не падал на
     *        TIME_WAIT-сокетах после перезапуска сервера.
     *     4. bind() на адрес и порт.
     *     5. listen() с SOMAXCONN.
     */
    void start(const ServerConfig& config);

    /**
     * @brief Закрывает сокет, если он открыт.
     */
    void closeSocket();
};
