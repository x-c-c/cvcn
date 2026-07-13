#pragma once
#include <netinet/in.h>
class ServerConfig
{
private:
	int	domain_		= AF_INET;
	int type_		= SOCK_STREAM;	//socket type
	int protocol_	= IPPROTO_TCP;
	in_addr_t addr_	= INADDR_ANY;
	int port_		= 55550;
public:
	ServerConfig() = default;
	ServerConfig(int domain, int type, int protocol, int port) :
	 domain_(domain), type_(type), protocol_(protocol), port_(port){};
	
	//==== getters ====
	// maybe not all of them is useful, some of them should be deleted
	int			getDomain()		const { return domain_; }
	int			getType()		const { return type_; }
	int			getProtocol()	const { return protocol_; }
	in_addr_t	getAddr()		const { return addr_; }
	int			getPort()		const { return port_; }
	
	//==== setters ====
	void		setAddr(in_addr_t newAddr)		{ addr_ = newAddr; }
	void		setPort(int newPort)			{ port_ = newPort; }	
};
