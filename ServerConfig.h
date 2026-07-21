/**
 * @file ServerConfig.h
 * @brief Конфигурация сервера (адрес, порт, параметры сокета).
 */

#pragma once
#include <netinet/in.h>

class ServerConfig
{
public:
	ServerConfig() = default;

	/**
	 * @brief Конструктор с ручным заданием параметров.
	 */
	ServerConfig(int domain, int type, int protocol, int port)
		: domain_(domain), type_(type), protocol_(protocol), port_(port) {}

	/** @name Геттеры */
	///@{
	int getDomain()		const { return domain_; }
	int getType()		const { return type_; }
	int getProtocol()	const { return protocol_; }
	in_addr_t getAddr()	const { return addr_; }
	int getPort()		const { return port_; }
	///@}

	/** @name Сеттеры */
	///@{
	void setAddr(in_addr_t newAddr) { addr_ = newAddr; }
	void setPort(int newPort)       { port_ = newPort; }
	///@}

private:
	int domain_		= AF_INET;       ///< Домен сокета (IPv4).
	int type_		= SOCK_STREAM;   ///< Тип сокета (потоковый).
	int protocol_	= IPPROTO_TCP;   ///< Протокол (TCP).
	in_addr_t addr_	= INADDR_ANY;  ///< Принимать соединения со всех интерфейсов.
	int port_		= 55550;         ///< Порт по умолчанию.
};
