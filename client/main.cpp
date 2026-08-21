#include <sys/socket.h>
#include <unistd.h>
#include "ClientConfig.h"

sockaddr_in makeServerAddr(const ClientConfig& config)
{
	sockaddr_in addr{};
	addr.sin_family			= config.getDomain();
	addr.sin_addr.s_addr	= config.getAddr();
	addr.sin_port			= htons(config.getPort());
	return addr;
}

int main()
{
	
	return 0;
}
