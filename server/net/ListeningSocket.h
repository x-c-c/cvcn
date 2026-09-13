#pragma once
#include "ServerConfig.h"
#include <sys/socket.h>
#include <unistd.h>

class ListeningSocket
{
public:
	ListeningSocket() = default;
	~ListeningSocket();

	ListeningSocket(const ListeningSocket&) = delete;
	ListeningSocket& operator=(const ListeningSocket&) = delete;

	int fileDescriptor() const { return serverSocketFD_; }

	/** @brief Создать, привязать к адресу и перевести в режим прослушивания. */
	void startListening(const ServerConfig& config);

	/** @brief Закрыть сокет, если он открыт. Идемпотентно. */
	void closeSocket();

private:
	int serverSocketFD_ = -1;
	sockaddr_in serverAddr_{};
	static constexpr int reuseAddrOption = 1;

	void initServerAddr(const ServerConfig& config);
};
