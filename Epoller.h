/**
 * @file Epoller.h
 * @brief Мультиплексирование ввода-вывода через epoll.
 *
 * Управляет серверным и клиентскими сокетами, передаёт события классам ClientSession.
 */

#pragma once
#include <sys/epoll.h>
#include <unordered_map>

class ClientSession;

class Epoller
{
public:
	/**
	 * @brief Конструктор – создаёт epoll-дескриптор.
	 */
	Epoller();
	/**
	 * @brief Деструктор – закрывает все клиентские сессии и epoll.
	 */
	~Epoller();

	/**
	 * @brief Запускает главный цикл обработки событий.
	 * @param serverSocketDescriptor файловый дескриптор слушающего сокета
	 */
	void startEpollLoop(int serverSocketDescriptor);

	/**
	 * @brief Останавливает главный цикл.
	 */
	void stopEpollLoop();

	/**
	 * @brief Изменяет набор отслеживаемых событий для fd.
	 * @param fileDescriptor файловый дескриптор
	 * @param events новая маска событий (EPOLLIN, EPOLLOUT и т.п.)
	 */
	void modifyFdEvents(int fileDescriptor, uint32_t events);

	/**
	 * @brief Закрывает клиентскую сессию и освобождает ресурсы.
	 * @param fileDescriptor файловый дескриптор клиента
	 */
	void closeClient(int fileDescriptor);

private:
	static constexpr int maxEvents = 1024;          ///< Максимальное число событий за один epoll_wait.
	int epollFileDescriptor_;                       ///< Файловый дескриптор epoll.
	bool running_ = true;                           ///< Флаг работы главного цикла.
	std::unordered_map<int, ClientSession*> sessions_; ///< Карта активных сессий (fd → объект).

	/**
	 * @brief Добавляет дескриптор в epoll с заданной маской событий.
	 */
	void addFdToEpoll(int fileDescriptor, uint32_t events);

	/**
	 * @brief Удаляет дескриптор из epoll.
	 */
	void removeFdFromEpoll(int fileDescriptor);

	/**
	 * @brief Принимает новое входящее соединение.
	 * @param serverSocketDescriptor слушающий сокет
	 */
	void handleNewConnection(int serverSocketDescriptor);
};
