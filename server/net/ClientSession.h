/**
 * @file    ClientSession.h
 * @brief   Одна клиентская сессия: чтение, разбор и обработка пакетов.
 *
 * @details
 *
 *   Владеет fd. Не владеет EventPoller и Database — только указатели.
 *   Живёт внутри SessionManager, который создаёт и удаляет объект.
 *   Все методы вызываются в потоке epoll-loop.
 * @warning После closeSession() объект удаляет SessionManager.
 * @see     SessionManager, EventPoller, PacketAssembler, PacketSender, Database
 */
#pragma once

#include "../protocol/PacketData.h"
#include "../protocol/PacketAssembler.h"
#include "../protocol/PacketSender.h"
#include <cstddef>
#include <cstdint>
#include <vector>

class EventPoller;
class Database;

class ClientSession
{
public:
    ClientSession(int socketFD, EventPoller* eventPoller, Database* db);
    ~ClientSession();                        ///< Закрывает fd, если ещё открыт.
	
	ClientSession(const ClientSession&) = delete;
    ClientSession& operator=(const ClientSession&) = delete;

    /**
     * @brief Обрабатывает событие EPOLLIN: читает байты и разбирает пакеты.
     *
     * @details
     *   Читает до 4 КБ за раз, отдаёт их PacketAssembler-у, затем в цикле
     *   вынимает из него целые пакеты и передаёт в processPacket.
     *   Если во время обработки сессия была закрыта (EOF, ошибка,
     *   DisconnectRequest) — выходит из цикла немедленно.
     *
     * @note    EAGAIN/EWOULDBLOCK — норма: данных пока нет.
     *          EINTR — прервано сигналом, epoll разбудит снова.
     */
    void handleRead();

    /**
     * @brief Обрабатывает событие EPOLLOUT: дренаж очереди отправки.
     */
    void handleWrite();

    /**
     * @brief Закрывает сессию.
     *
     * @details
     *   Первый вызов закрывает fd и выставляет closed_ = true.
     *   Повторные вызовы ничего не делают. Объект не удаляется —
     *   это делает SessionManager в eraseIfClosed.
     */
    void closeSession();

    /// @brief Дескриптор сокета этой сессии.
    int getSocketDescriptor() const { return socketFD_; }

    /// @brief Закрыта ли сессия.
    bool isClosed() const { return closed_; }

    /// @brief Присваивается в handlePacket(ConnectRequestData).
    uint32_t sessionID_ = 0;

private:
    /// Размер временного буфера для одного recv.
    static constexpr std::size_t TEMP_BUFFER_SIZE = 4096;

    int socketFD_;                  ///< Владеет. Закрывается в closeSession().
    bool closed_ = false;
    EventPoller* eventPoller_;      ///< Не владеет. Живёт дольше сессии.
    Database* db_;                  ///< Не владеет. Общая на весь сервер.
    PacketAssembler assembler_;     ///< Владеет. Буфер незавершённых пакетов.
    PacketSender sender_;           ///< Владеет. Очередь исходящих пакетов.

    /**
     * @brief Диспетчер по типу тела пакета.
     *
     * @param header Разобранный заголовок (тип, messageID, sessionID, длина).
     * @param body   Тело пакета — сырые байты после заголовка.
     */
    void processPacket(const PacketHeaderRaw& header,
                       const std::vector<uint8_t>& body);

    // По одной перегрузке на каждый тип входящего пакета 
    // Перегрузка выбирается компилятором по типу третьего аргумента.

    /// @brief ConnectRequest: назначает sessionID и отправляет ConnectResponse.
    void handlePacket(uint32_t messageID, uint32_t sessionID,
                      const ConnectRequestData& data);

    /// @brief RegisterRequest: addUser в БД, отправляет RegisterResponse.
    void handlePacket(uint32_t messageID, uint32_t sessionID,
                      const RegisterRequestData& data);

    /// @brief AuthRequest: сверяет хэш, отправляет AuthResponse.
    void handlePacket(uint32_t messageID, uint32_t sessionID,
                      const AuthRequestData& data);

    /// @brief MessageSend: пока только логирует, рассылка — TODO.
    void handlePacket(uint32_t messageID, uint32_t sessionID,
                      const MessageSendData& data);

    /// @brief DisconnectRequest: закрывает сессию.
    void handlePacket();
};
