#pragma once
#include <netinet/in.h>
class ClientConfig
{
public:
	ClientConfig() = default;
	ClientConfig(int domain, int type, int protocol, int port): domain_(domain), type_(type), protocol_(protocol), port_(port) {}
	
	int getDomain()		const { return domain_; }
	int getType()		const { return type_; }
	int getProtocol()	const { return protocol_; }
	in_addr_t getAddr()	const { return addr_; }
	int getPort()		const { return port_; }
	
	void setAddr(in_addr_t newAddr) { addr_ = newAddr; }
	void setPort(int newPort)       { port_ = newPort; }

private:
	int domain_		= AF_INET;					///< Домен сокета (IPv4).
	int type_		= SOCK_STREAM;				///< Тип сокета (потоковый).
	int protocol_	= IPPROTO_TCP;				///< Протокол (TCP).
	in_addr_t addr_	= INADDR_LOOPBACK;			///< Принимать соединения со всех интерфейсов.
	//in_addr_t addr_ = inet_addr("127.0.0.1");
	int port_		= 55550;					///< Порт по умолчанию.
};
