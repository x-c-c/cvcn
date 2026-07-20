#pragma once
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class ClientSession
{
private:
	int socketFd_;
	bool closed_ = false;
	std::vector<uint8_t> readBuffer_;
	static constexpr size_t MAX_BUFFER_SIZE = 64 * 1024;	// 64 kB
	
	void tryExtractPackets();
public:
	explicit ClientSession(int socketFd);
	~ClientSession();
	
	void handleRead();
	void closeSession();

	int getSocketFd() const	{ return socketFd_; }
	bool isClosed() const { return closed_;	}
};
