/**
 * @file    ServerConfig.h
 * @brief   Параметры серверного сокета: домен, тип, протокол, адрес, порт.
 *
 * @details
 *   Класс ничего не делает — только хранит значения, которые нужны
 *   ListeningSocket для вызовов socket(), bind(), listen().
 *   Значения по умолчанию рассчитаны на локальный запуск IPv4/TCP
 *   на порту 55550. Порт может быть переопределён через PortSelector
 *   при старте сервера.
 * @see     ListeningSocket, PortSelector
 */

#pragma once
#include <netinet/in.h>

/**
 * @brief Хранит настройки для создания слушающего сокета.
 *
 * @details
 *   Поля приватные, доступ через геттеры/сеттеры.
 */
class ServerConfig
{
public:
    /**
     * @brief Конструктор по умолчанию: IPv4, TCP, INADDR_ANY, порт 55550.
     */
    ServerConfig() = default;

    /**
     * @brief Полностью параметризованный конструктор.
     *
     * @param domain   Домен сокета: AF_INET (IPv4).
     * @param type     Тип сокета: SOCK_STREAM (TCP).
     * @param protocol Протокол: IPPROTO_TCP.
     * @param port     Порт (1..65535).
     */
    ServerConfig(int domain, int type, int protocol, int port)
        : domain_(domain), type_(type), protocol_(protocol), port_(port) {}

    /// @brief Домен сокета (AF_INET).
    int getDomain() const { return domain_; }

    /// @brief Тип сокета (SOCK_STREAM).
    int getType() const { return type_; }

    /// @brief Протокол (IPPROTO_TCP).
    int getProtocol() const { return protocol_; }

    /// @brief Адрес, на котором слушать (INADDR_ANY).
    in_addr_t getAddr() const { return addr_; }

    /// @brief Номер порта (1..65535).
    int getPort() const { return port_; }

    /**
     * @brief Меняет адрес прослушивания.
     * @param newAddr INADDR_ANY или IP в сетевом порядке байт.
     */
    void setAddr(in_addr_t newAddr) { addr_ = newAddr; }

    /**
     * @brief Меняет порт.
     * @param newPort Новый номер порта.
     * @note Валидацию диапазона (1..65535) делает вызывающий код
     *       (PortSelector).
     */
    void setPort(int newPort) { port_ = newPort; }

private:
    int domain_     = AF_INET;      ///< Домен сокета: IPv4.
    int type_       = SOCK_STREAM;  ///< Тип сокета: потоковый (TCP).
    int protocol_   = IPPROTO_TCP;  ///< Протокол: TCP.
    in_addr_t addr_ = INADDR_ANY;   ///< Принимать соединения со всех интерфейсов.
    int port_       = 55550;        ///< Порт по умолчанию.
};
