/**
 * @file    Model.cpp
 * @brief   Реализация Model: асинхронный TCP-клиент с разбором
 *          входящего потока на пакеты.
 *
 * @details
 *   Модель работает асинхронно: connectToHost не блокирует,
 *   slotReadyRead вызывается Qt-ом по мере поступления данных.
 *   Все действия — в главном потоке, где крутится QApplication::exec.
 *
 *   Исходящие пакеты собираются целиком через PacketBuilder и
 *   уходят в socket_->write(). Входящий поток режется на пакеты
 *   вручную, потому что QTcpSocket не знает про наши messageLen.
 *
 * @see Model.h
 */

#include <QDebug>
#include "./Model.h"
#include "../protocol/PacketData.h"
#include "../protocol/PacketBuilder.h"
#include "../protocol/PacketParser.h"

Model::Model(QObject* parent):
    QObject(parent),
    socket_(nullptr)
{
    socket_ = new QTcpSocket(this);
    connect(socket_, &QTcpSocket::connected,     this, &Model::slotConnected);
    connect(socket_, &QTcpSocket::errorOccurred, this, &Model::slotSocketError);
    connect(socket_, &QTcpSocket::readyRead,     this, &Model::slotReadyRead);
}

void Model::connectToServer(const QString& address, const quint16 port)
{
    // Асинхронный вызов: управление вернётся сразу, результат
    // придёт сигналом connected или errorOccurred.
    socket_->connectToHost(address, port);
}

void Model::slotConnected()
{
    // Пока просто отметка в отладке. Сюда стоит поставить
    // отправку ConnectRequest, если сервер ждёт его до других
    // запросов. Сейчас протокол работает и без него: sessionID
    // на клиенте остаётся 0, а сервер его игнорирует.
    qDebug() << "connected";
}

void Model::slotSocketError(QAbstractSocket::SocketError error)
{
    // errorString() — человекочитаемое описание, error — код.
    // Логируем оба, потом уведомляем подписчиков через сигнал.
    qDebug() << "Socket error:" << socket_->errorString()
             << "(code " << error << " )";

    emit errorOccurred(socket_->errorString());
}

void Model::slotReadyRead()
{
    // readAll() забирает всё, что накопилось в буфере сокета.
    // QByteArray и std::vector<uint8_t> неявно несовместимы,
    // поэтому копируем поэлементно через insert.
    const QByteArray chunk = socket_->readAll();
    receiveBuffer_.insert(receiveBuffer_.end(), chunk.begin(), chunk.end());

    // Крутимся, пока в буфере есть хотя бы целый заголовок.
    while (receiveBuffer_.size() >= sizeof(PacketHeaderRaw))
    {
        // Разбираем заголовок. Функция вернёт false, если байт
        // не хватило (но мы это уже проверили) — страховка.
        PacketHeaderRaw header;
        if (!PacketParser::deserializeHeader(receiveBuffer_, header))
            return;

        // Проверяем, что тела тоже достаточно. Если нет —
        // выходим и ждём следующий readyRead. Заголовок при этом
        // уже разобран, но мы его не сохраняем — перечитаем
        // при следующем заходе. Это ок для маленьких пакетов,
        // но лишняя работа; при оптимизации стоит запоминать
        // «ожидаемую длину».
        const std::size_t totalSize = sizeof(PacketHeaderRaw) + header.messageLen;
        if (receiveBuffer_.size() < totalSize)
            return;

        // Вырезаем тело пакета — отдельным вектором.
        // Пока оно не используется, только для симметрии с
        // PacketAssembler и на будущее, когда появится обработка.
        std::vector<uint8_t> body(
            receiveBuffer_.begin() + sizeof(PacketHeaderRaw),
            receiveBuffer_.begin() + totalSize);

        // Убираем пакет из буфера целиком (заголовок + тело).
        // erase сдвигает оставшиеся байты в начало — O(n) на
        // хвост, но для потока пакетов это приемлемо.
        receiveBuffer_.erase(receiveBuffer_.begin(),
                             receiveBuffer_.begin() + totalSize);

        // Пока только логирование. Место для dispatch-а по
        // header.type: обработка RegisterResponse, AuthResponse,
        // MessageReceive и т.п. — TODO.
        qDebug() << "Packet type:" << static_cast<int>(header.type)
                 << "messageID:"   << header.messageID
                 << "sessionID:"   << header.sessionID
                 << "len:"         << header.messageLen;
    }
}

void Model::sendPacket(const std::vector<uint8_t>& packet)
{
    // Проверка состояния сокета: если не подключены — отправлять
    // некуда. QTcpSocket::write в этом состоянии обычно просто
    // игнорирует данные, но лучше явно уведомить.
    if (socket_->state() != QAbstractSocket::ConnectedState)
    {
        emit errorOccurred(QStringLiteral("Not connected to server"));
        return;
    }

    // Копирование в QByteArray обязательно: write() может
    // буферизовать данные, а оригинальный vector может
    // умереть раньше, чем Qt их отправит.
    const QByteArray data(reinterpret_cast<const char*>(packet.data()),
                          static_cast<int>(packet.size()));

    const qint64 bytesWritten = socket_->write(data);
    if (bytesWritten == -1)
        emit errorOccurred(socket_->errorString());
}

void Model::sendRegRequest(const QString& username, const QString& password)
{
    // Заполняем структуру из QString. toStdString — потому что
    // PacketData работает с std::string, а не с QString.
    RegisterRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();

    // Собираем пакет: заголовок + тело. messageID_ гарантированно
    // уникален в рамках сессии, sessionID_ пока 0.
    const auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);

    sendPacket(packet);

    // Инкремент после отправки — следующий запрос получит новый ID.
    increaseMessageID();
}

void Model::sendAuthRequest(const QString& username, const QString& password)
{
    // Симметрично sendRegRequest, но с AuthRequestData.
    AuthRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();

    const auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}

/*
void Model::sendDelRequest(const QString& username, const QString& password)
{
    DeleteRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    const auto packet = PacketBuilder::buildPacket(messageID_, sessionID_, payload);
    sendPacket(packet);
    increaseMessageID();
}
*/

void Model::increaseMessageID()
{
    // Пост-инкремент: текущее значение уже использовано в пакете,
    // следующее начнётся с +1. При переполнении uint32_t
    // произойдёт wrap-around — сервер это не заметит, потому что
    // messageID используется только для сопоставления запроса
    // и ответа, а не как монотонная метка.
    ++messageID_;
}
