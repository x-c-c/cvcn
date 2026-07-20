#include "ClientSession.h"
#include "Packets.h"


ClientSession::ClientSession(int socketFd): socketFd_(socketFd){}
ClientSession::~ClientSession()
{
	if (!closed_)
		closeSession();
}









void ClientSession::closeSession()
{
	if (closed_)
		return;
	closed_ = true;
	close(socketFd_);
	std::cout << "[Client " << socketFd_ << "] session closed" << std::endl;
}
