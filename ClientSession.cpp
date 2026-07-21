/**
 * @file ClientSession.cpp
 * @brief Реализация сессии клиента.
 */

#include "ClientSession.h"
#include "Epoller.h"
#include "Serializer.h"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>

ClientSession::ClientSession(int socketDescriptor, Epoller* epoller)
	: socketDescriptor_(socketDescriptor), epoller_(epoller)
{
}

ClientSession::~ClientSession()
{
	if (!closed_)
		closeSession();
}

void ClientSession::handleRead()
{
	uint8_t tempBuffer[4096];
	ssize_t bytesRead = recv(socketDescriptor_, tempBuffer, sizeof(tempBuffer), 0);
	if (bytesRead > 0)
	{
		size_t oldSize = readBuffer_.size();
		readBuffer_.resize(oldSize + bytesRead);
		std::memcpy(readBuffer_.data() + oldSize, tempBuffer, bytesRead);
		tryExtractPackets();
	}
	else if (bytesRead == 0)
	{
		std::cout << "[Client " << socketDescriptor_ << "] connection closed by peer" << std::endl;
		closeSession();
	}
	else
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			perror("recv error");
			closeSession();
		}
	}
}

/**
 * @brief Пытается извлечь один полный пакет из буфера.
 * @param header заполняемый заголовок (хостовый порядок)
 * @param body   тело пакета
 * @return true, если пакет успешно извлечён
 */
bool ClientSession::extractPacket(PacketHeaderRaw& header, std::vector<uint8_t>& body)
{
	if (readBuffer_.size() < sizeof(PacketHeaderRaw))
		return false;

	std::memcpy(&header, readBuffer_.data(), sizeof(header));
	header.type       = ntohs(header.type);
	header.messageID  = ntohl(header.messageID);
	header.sessionID  = ntohl(header.sessionID);
	header.messageLen = ntohs(header.messageLen);

	size_t totalSize = sizeof(PacketHeaderRaw) + header.messageLen;
	if (readBuffer_.size() < totalSize)
		return false;

	body.assign(readBuffer_.begin() + sizeof(PacketHeaderRaw),
				readBuffer_.begin() + totalSize);
	readBuffer_.erase(readBuffer_.begin(), readBuffer_.begin() + totalSize);
	return true;
}

/**
 * @brief Извлекает все доступные полные пакеты и отправляет на обработку.
 */
void ClientSession::tryExtractPackets()
{
	PacketHeaderRaw header;
	std::vector<uint8_t> body;
	while (extractPacket(header, body))
	{
		processPacket(header, body);
		if (closed_)
			return;
	}
}

/**
 * @brief Определяет тип пакета и вызывает соответствующий обработчик.
 */
void ClientSession::processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
	switch (static_cast<PacketType>(header.type))
	{
	case PacketType::ConnectRequest:
		{
			ConnectRequestPacket packet;
			Serializer::parseConnectRequestPacket(body, packet);
			handleConnectRequestPacket(header.messageID, header.sessionID);
		}
		break;

	case PacketType::RegisterRequest:
		{
			RegisterRequestPacket packet;
			if (Serializer::parseRegisterRequestPacket(body, packet))
				handleRegisterRequestPacket(header.messageID, header.sessionID, packet);
		}
		break;

	case PacketType::AuthRequest:
		{
			AuthRequestPacket packet;
			if (Serializer::parseAuthRequestPacket(body, packet))
				handleAuthRequestPacket(header.messageID, header.sessionID, packet);
		}
		break;

	case PacketType::MessageSend:
		{
			MessageSendPacket packet;
			if (Serializer::parseMessageSendPacket(body, packet))
				handleMessageSendPacket(header.messageID, header.sessionID, packet);
		}
		break;

	case PacketType::DisconnectRequest:
		handleDisconnectRequestPacket();
		break;

	default:
		std::cerr << "[Client " << socketDescriptor_ << "] unknown packet type 0x"
				  << std::hex << header.type << std::dec << std::endl;
		break;
	}
}

/**
 * @brief Регистрирует интерес к EPOLLOUT и взводит флаг ожидания.
 */
void ClientSession::armWriteNotification()
{
	epoller_->modifyFdEvents(socketDescriptor_, EPOLLIN | EPOLLOUT);
	writePending_ = true;
}

/**
 * @brief Пытается отправить все сообщения из очереди.
 *        При блокировке сокета регистрирует EPOLLOUT.
 */
void ClientSession::flushSendQueue()
{
	while (!sendQueue_.empty())
	{
		std::vector<uint8_t>& buffer = sendQueue_.front();
		ssize_t sent = send(socketDescriptor_, buffer.data(), buffer.size(), MSG_NOSIGNAL);
		if (sent < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				armWriteNotification();
				return;
			}
			perror("send error");
			closeSession();
			return;
		}
		if (static_cast<size_t>(sent) < buffer.size())
		{
			buffer.erase(buffer.begin(), buffer.begin() + sent);
			armWriteNotification();
			return;
		}
		sendQueue_.pop_front();
	}
	epoller_->modifyFdEvents(socketDescriptor_, EPOLLIN);
	writePending_ = false;
}

/**
 * @brief Ставит данные в очередь отправки и инициирует запись.
 */
void ClientSession::sendResponse(const std::vector<uint8_t>& data)
{
	if (closed_)
		return;
	sendQueue_.push_back(data);
	if (!writePending_)
		flushSendQueue();
}

/**
 * @brief Вызывается Epoller'ом при событии EPOLLOUT.
 */
void ClientSession::handleWrite()
{
	if (closed_)
		return;
	writePending_ = false;
	flushSendQueue();
}

// ----------------------------------------------------------------
// Обработчики команд (заглушки, позже подключатся к БД)
// ----------------------------------------------------------------

/**
 * @brief Обрабатывает запрос подключения: генерирует идентификатор сессии и отвечает.
 */
void ClientSession::handleConnectRequestPacket(uint32_t messageID, uint32_t sessionID)
{
	uint32_t newSessionID = sessionID ? sessionID : static_cast<uint32_t>(socketDescriptor_);
	auto response = Serializer::buildConnectResponsePacket(messageID, newSessionID);
	sendResponse(response);
}

/**
 * @brief Обрабатывает запрос регистрации (пока всегда успешно).
 */
void ClientSession::handleRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet)
{
	RegisterResponsePacket responsePkt;
	responsePkt.success = 1;
	auto response = Serializer::buildRegisterResponsePacket(messageID, sessionID, responsePkt);
	sendResponse(response);
}

/**
 * @brief Обрабатывает запрос аутентификации (пока всегда успешно).
 */
void ClientSession::handleAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet)
{
	AuthResponsePacket responsePkt;
	responsePkt.success = 1;
	auto response = Serializer::buildAuthResponsePacket(messageID, sessionID, responsePkt);
	sendResponse(response);
}

/**
 * @brief Выводит полученное сообщение в лог.
 */
void ClientSession::handleMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet)
{
	std::cout << "[Client " << socketDescriptor_ << "] Message from " << packet.senderID
			  << " to chat " << packet.chatID << ": " << packet.text << std::endl;
}

/**
 * @brief Обрабатывает запрос отключения – закрывает сессию.
 */
void ClientSession::handleDisconnectRequestPacket()
{
	closeSession();
}

/**
 * @brief Закрывает клиентский сокет и помечает сессию закрытой.
 */
void ClientSession::closeSession()
{
	if (closed_)
		return;
	closed_ = true;
	close(socketDescriptor_);
	std::cout << "[Client " << socketDescriptor_ << "] session closed" << std::endl;
}
