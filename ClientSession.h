/**
 * @file ClientSession.h
 * @brief Сессия одного подключённого клиента.
 *
 * Выполняет буферизацию входящих данных, выделение полных пакетов,
 * их обработку и асинхронную отправку ответов.
 */

#pragma once
#include "Packets.h"
#include <vector>
#include <cstdint>
#include <deque>
#include <sys/socket.h>

class Epoller;

class ClientSession
{
public:
	/**
	 * @brief Конструктор.
	 * @param socketDescriptor клиентский сокет
	 * @param epoller          указатель на Epoller для взаимодействия с циклом событий
	 */
	ClientSession(int socketDescriptor, Epoller* epoller);
	~ClientSession();

	/**
	 * @brief Вызывается при готовности сокета к чтению (EPOLLIN).
	 */
	void handleRead();

	/**
	 * @brief Закрывает сокет и помечает сессию закрытой.
	 */
	void closeSession();

	int getSocketDescriptor() const { return socketDescriptor_; }
	bool isClosed() const            { return closed_; }

private:
	friend class Epoller;   ///< Epoller имеет доступ к handleWrite и closeSession.

	int socketDescriptor_;                           ///< Файловый дескриптор клиентского сокета.
	bool closed_ = false;                            ///< Закрыта ли сессия.
	Epoller* epoller_;                               ///< Указатель на Epoller для изменения событий.
	std::vector<uint8_t> readBuffer_;                ///< Буфер накопления входящих данных.
	static constexpr size_t maxBufferSize = 64 * 1024; ///< Предельный размер буфера.
	std::deque<std::vector<uint8_t>> sendQueue_;     ///< Очередь исходящих сообщений.
	bool writePending_ = false;                      ///< Ожидаем готовности сокета к записи.

	void tryExtractPackets();
	bool extractPacket(PacketHeaderRaw& header, std::vector<uint8_t>& body);
	void processPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
	void sendResponse(const std::vector<uint8_t>& data);
	void handleWrite();
	void flushSendQueue();
	void armWriteNotification();

	// Обработчики команд
	void handleConnectRequestPacket(uint32_t messageID, uint32_t sessionID);
	void handleRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet);
	void handleAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet);
	void handleMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet);
	void handleDisconnectRequestPacket();
};
