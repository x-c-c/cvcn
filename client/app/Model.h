/**
 * @file    Model.h
 * @brief   Сетевая модель клиента: транспорт + минимальный разбор
 *          входящего потока на пакеты.
 *
 * @details
 *   Класс из MVC (Model-View-Controller): отвечает за связь
 *   с сервером и за отправку запросов. View (AccountDialog) и
 *   Controller друг о друге знают, но Model — только о сокете.
 *
 *   Владеет QTcpSocket-ом (создаётся в конструкторе, parent = this,
 *   удалится автоматически). Асинхронный: connectToHost не блокирует,
 *   ответы приходят через сигналы readyRead / connected /
 *   errorOccurred.
 *
 *   Отправка:
 *     sendRegRequest / sendAuthRequest → PacketBuilder → sendPacket
 *     → socket_->write(). Пакет собирается целиком в std::vector,
 *     потом конвертируется в QByteArray для QTcpSocket.
 *
 *   Приём:
 *     slotReadyRead вызывается Qt-ом по мере поступления байт,
 *     копит их в receiveBuffer_ и режет на пакеты по заголовку.
 *     Разбор конкретного тела сейчас не делается — только логируется
 *     заголовок. Место для будущей обработки ответов сервера.
 *
 * @note    Все операции — в главном потоке Qt (event-loop
 *          QApplication). Никаких std::thread/QThread внутри нет.
 * @warning receiveBuffer_ не ограничен сверху. Если сервер пришлёт
 *          заголовок с messageLen = 60000 и оборвёт соединение,
 *          буфер будет висеть до закрытия сокета. Лимит — на будущее.
 * @see     Controller, PacketBuilder, PacketParser, PacketData
 */

#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QAbstractSocket>
#include <vector>
#include <cstdint>

class Model : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Создаёт сокет и подключает его сигналы к слотам модели.
     *
     * @param parent Родитель в дереве Qt. Обычно nullptr — модель
     *               живёт в main и не привязана к виджету.
     *
     * @note QTcpSocket получает parent = this, поэтому удалится
     *       автоматически вместе с Model. Явный delete в деструкторе
     *       не нужен.
     */
    explicit Model(QObject* parent = nullptr);

    /// @brief Деструктор по умолчанию: всё чистит QObject-родитель.
    ~Model() = default;

    /**
     * @brief Начинает TCP-подключение к серверу.
     *
     * @param address IP-адрес или имя хоста.
     * @param port    Порт (host order).
     *
     * @details
     *   Асинхронный вызов: возвращается немедленно. Успех или
     *   ошибка придут через сигналы QTcpSocket::connected /
     *   errorOccurred, которые уже подключены к слотам модели.
     */
    void connectToServer(const QString& address, const quint16 port);

    /**
     * @brief Отправляет запрос регистрации.
     *
     * @param username Логин нового пользователя.
     * @param password Пароль (сейчас в открытом виде).
     *
     * @details
     *   Собирает RegisterRequestData → PacketBuilder::buildPacket
     *   → sendPacket. После успешной отправки инкрементирует
     *   messageID_, чтобы следующий запрос имел новый ID.
     *
     * @note Ответ сервера (RegisterResponse) сейчас не обрабатывается —
     *       разбирается только заголовок в slotReadyRead.
     */
    void sendRegRequest(const QString& username, const QString& password);

    /**
     * @brief Отправляет запрос аутентификации.
     *
     * @param username Логин.
     * @param password Пароль (сейчас в открытом виде).
     *
     * @details
     *   Симметрично sendRegRequest: собирает AuthRequestData,
     *   отправляет, увеличивает messageID_.
     */
    void sendAuthRequest(const QString& username, const QString& password);

    // void sendDelRequest(const QString& username, const QString& password);

    /**
     * @brief Отправляет готовый пакет в сокет.
     *
     * @param packet Готовый к отправке пакет: заголовок + тело,
     *               все многобайтовые поля в network order.
     *
     * @details
     *   Проверяет, что сокет в состоянии ConnectedState. Если нет —
     *   эмитит errorOccurred и выходит. Иначе конвертирует
     *   std::vector<uint8_t> в QByteArray и пишет в сокет.
     *
     * @note Копия packet в QByteArray нужна, потому что QTcpSocket
     *       может буферизовать данные асинхронно — оригинальный
     *       vector к моменту отправки уже мог бы умереть.
     */
    void sendPacket(const std::vector<uint8_t>& packet);

signals:
    /**
     * @brief Сигнал об ошибке — сетевой или логической.
     *
     * @param message Текст ошибки от QTcpSocket или собственное
     *                сообщение модели.
     *
     * @details
     *   Эмитится из slotSocketError (после ошибки сокета) и из
     *   sendPacket (если попытка отправки без соединения).
     *   Подписчики — Controller или UI, которые решают, что показать
     *   пользователю.
     */
    void errorOccurred(const QString& message);

private:
    QTcpSocket* socket_;                    ///< Владеет. parent = this.
    uint32_t messageID_ = 0;                ///< Инкрементируется после каждой отправки.
    uint32_t sessionID_ = 0;                ///< Назначается сервером в ConnectResponse (пока не используется).
    std::vector<uint8_t> receiveBuffer_;    ///< Владеет. Накопитель входящих байт.

    /**
     * @brief Инкрементирует messageID_ после отправки.
     * @details Вызывается в конце sendRegRequest/sendAuthRequest.
     */
    void increaseMessageID();

private slots:
    /**
     * @brief Реакция на QTcpSocket::connected.
     * @details Сейчас только пишет в qDebug, что соединение
     *          установлено. Место для будущей отправки ConnectRequest.
     */
    void slotConnected();

    /**
     * @brief Разбирает входящие байты на пакеты.
     *
     * @details
     *   Вызывается Qt-ом по сигналу QTcpSocket::readyRead, когда
     *   пришли новые байты. Алгоритм:
     *     1. Забрать всё из сокета через readAll().
     *     2. Добавить в receiveBuffer_.
     *     3. В цикле: если в буфере хватает на заголовок —
     *        разобрать его; если хватает и на тело —
     *        вырезать пакет из буфера.
     *
     *   Разбор конкретного тела пакета здесь не делается: только
     *   логируется header. Обработка ответов сервера — TODO.
     *
     * @note Если пакет пришёл частично, буфер сохранит остаток,
     *       и следующий readyRead вызовет слот снова.
     */
    void slotReadyRead();

    /**
     * @brief Реакция на QTcpSocket::errorOccurred.
     *
     * @param error Код ошибки (enum QAbstractSocket).
     *
     * @details
     *   Логирует errorString() и код, затем эмитит errorOccurred
     *   для подписчиков. Не пытается переподключиться — это
     *   ответственность выше.
     */
    void slotSocketError(QAbstractSocket::SocketError error);
};

#endif // MODEL_H
