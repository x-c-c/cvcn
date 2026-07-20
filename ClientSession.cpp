#include "ClientSession.h"
#include "Packets.h"


ClientSession::ClientSession(int socketFd): socketFd_(socketFd){}
ClientSession::~ClientSession()
{
	if (!closed_)
		closeSession();
}

void ClientSession::handleRead()
{
	uint8_t tmp[4096];
	ssize_t bytesRead  = recv(socketFd_, tmp, sizeof(tmp), 0);
	if (bytesRead  > 0)
	{
		// Добавляем прочитанные байты в буфер
		size_t oldSize = readBuffer_.size();
		readBuffer_.resize(oldSize + bytesRead);
		std::memcpy(readBuffer_.data() + oldSize, tmp, sizeof(bytesRead));
		
		// Пытаемся выделить целые пакеты
		tryExtractPackets();
	}
	else if(bytesRead == 0)
	{
		// Клиент корректно закрыл соединение
		std::cout << "[Client " << socketFd_ << "] connection closed by peer" << std::endl;
		closeSession();
	}
	else
	{
		// Ошибка чтения
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			perror("recv error");
			closeSession();
		}
	}
}

void ClientSession::tryExtractPackets()
{
	while(true)
	{
		if (readBuffer_.size() < sizeof(PacketHeaderRaw))	// Минимальный размер заголовка: 12 байт (на 20.07.2026)
			break;
		// Читаем заголовок
		PacketHeaderRaw header;
		std::memcpy(&header, readBuffer_.data(), sizeof(header));
		uint16_t messageLen = ntohs(header.messageLen_);
		size_t totalSize = sizeof(PacketHeaderRaw) + messageLen;
		
		// Если прочитано байт меньше чем требуется для выделения целого пакета, то ждём ещё
		if (readBuffer_.size() < totalSize)
			break;
		
		// Извлекаем полный пакет отдельный вектор
		std::vector<uint8_t> packet(readBuffer_.begin(), readBuffer_.begin() + totalSize);
		
		// Удаляем байты, записанные в отдельный вектор, из буфера
		readBuffer_.erase(readBuffer_.begin() + totalSize);
				
		// потом тут будет роутер
		uint16_t type = ntohs(header.type_);
		std::cout << "[Client " << socketFd_ << "] packet extracted, type=0x"
				  << std::hex << type << std::dec
				  << ", msgID=" << ntohl(header.messageID_)
				  << ", bodyLen=" << messageLen << std::endl;
	
	}
}





void ClientSession::closeSession()
{
	if (closed_)
		return;
	closed_ = true;
	close(socketFd_);
	std::cout << "[Client " << socketFd_ << "] session closed" << std::endl;
}
