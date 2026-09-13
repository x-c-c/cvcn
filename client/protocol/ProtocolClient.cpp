#include "ProtocolClient.h"
#include "Connection.h"
#include "PacketBuilder.h"
#include "PacketParser.h"
#include "Logger.h"
#include <exception>

ProtocolClient::ProtocolClient(Connection* connection, QObject* parent):
    QObject(parent),
    connection_(connection)
{
    connect(connection_, &Connection::signalConnected,         this, &ProtocolClient::slotConnected);
    connect(connection_, &Connection::signalDisconnected,      this, &ProtocolClient::slotDisconnected);
    connect(connection_, &Connection::signalErrorOccurred,     this, &ProtocolClient::slotTransportError);
    connect(connection_, &Connection::signalRawPacketReceived, this, &ProtocolClient::slotRawPacketReceived);
}

void ProtocolClient::connectToServer(const QString& address, quint16 port)
{
    connection_->connectToServer(address, port);
}

void ProtocolClient::slotConnected()
{
    LOG_INFO("connected");
    emit signalConnected();
}

void ProtocolClient::slotDisconnected()
{
    LOG_INFO("disconnected");
    emit signalDisconnected();
}

void ProtocolClient::slotTransportError(ErrorKind kind, const QString& errorString)
{
    LOG_ERROR("{}", errorString.toStdString());
    emit signalErrorOccurred(kind, errorString);
}

void ProtocolClient::slotRawPacketReceived(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
    // Никакой парсер не должен уронить приложение. Ловим все исключения,
    // которые могут прилететь из std::string, std::vector, Qt-преобразований.
    // При ошибке эмитим Protocol error и продолжаем работать.
    try
    {
        processIncomingPacket(header, body);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception while parsing packet type 0x{:X}: {}",
                  header.type, e.what());
        emit signalErrorOccurred(ErrorKind::Protocol,
                                 QString::fromUtf8(e.what()));
    }
    catch (...)
    {
        LOG_ERROR("Unknown exception while parsing packet type 0x{:X}",
                  header.type);
        emit signalErrorOccurred(ErrorKind::Protocol,
                                 QStringLiteral("Unknown parse error"));
    }
}

void ProtocolClient::processIncomingPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body)
{
    switch (static_cast<PacketType>(header.type))
    {
    case PacketType::RegisterResponse:
    {
        RegisterResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit signalRegistrationFinished(resp.success);
        else
            emit signalErrorOccurred(ErrorKind::Protocol,
                QStringLiteral("Bad RegisterResponse"));
        break;
    }
    case PacketType::AuthResponse:
    {
        AuthResponseData resp{};
        if (PacketParser::parseData(body, resp))
        {
            if (resp.success)
                sessionID_ = header.sessionID;
            emit signalAuthFinished(resp.success, sessionID_);
        }
        else
            emit signalErrorOccurred(ErrorKind::Protocol,
                QStringLiteral("Bad AuthResponse"));
        break;
    }
    case PacketType::DeleteResponse:
    {
        DeleteResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit signalDeleteFinished(resp.success);
        else
            emit signalErrorOccurred(ErrorKind::Protocol,
                QStringLiteral("Bad DeleteResponse"));
        break;
    }
    case PacketType::FindUserResponse:
    {
        FindUserResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit signalUsersFound(resp.usernames);
        else
            emit signalErrorOccurred(ErrorKind::Protocol,
                QStringLiteral("Bad FindUserResponse"));
        break;
    }
    case PacketType::CreateChatResponse:
    {
        CreateChatResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit signalChatCreated(resp.success, resp.chatID,
                                   QString::fromStdString(resp.peerUsername));
        else
            emit signalErrorOccurred(ErrorKind::Protocol,
                QStringLiteral("Bad CreateChatResponse"));
        break;
    }
    case PacketType::ChatListResponse:
    {
        ChatListResponseData resp{};
        if (PacketParser::parseData(body, resp))
            emit signalChatListReceived(resp.chats);
        else
            emit signalErrorOccurred(ErrorKind::Protocol,
                QStringLiteral("Bad ChatListResponse"));
        break;
    }
    case PacketType::MessageReceive:
    {
        MessageReceiveData recv{};
        if (PacketParser::parseData(body, recv))
        {
            emit signalMessageReceived(
                QString::fromStdString(recv.senderUsername),
                recv.chatID,
                QString::fromStdString(recv.text));
        }
        else
            emit signalErrorOccurred(ErrorKind::Protocol,
                QStringLiteral("Bad MessageReceive"));
        break;
    }
    default:
        LOG_WARN("Unknown packet type: {}", static_cast<int>(header.type));
        emit signalErrorOccurred(ErrorKind::Protocol,
            QStringLiteral("Unknown packet type %1").arg(static_cast<int>(header.type)));
        break;
    }
}

void ProtocolClient::sendPacket(const std::vector<uint8_t>& packet)
{
    connection_->send(packet);
}

void ProtocolClient::sendRegRequest(const QString& username, const QString& password)
{
    RegisterRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendAuthRequest(const QString& username, const QString& password)
{
    AuthRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendDeleteRequest(const QString& username, const QString& password)
{
    DeleteRequestData payload;
    payload.username = username.toStdString();
    payload.password = password.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendMessage(uint32_t chatID, const QString& text)
{
    MessageSendData payload;
    payload.chatID = chatID;
    payload.text   = text.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendFindUserRequest(const QString& query)
{
    FindUserRequestData payload;
    payload.query = query.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendCreateChatRequest(const QString& peerUsername)
{
    CreateChatRequestData payload;
    payload.peerUsername = peerUsername.toStdString();
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendChatListRequest()
{
    ChatListRequestData payload;
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::sendDisconnectRequest()
{
    DisconnectRequestData payload;
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

void ProtocolClient::increaseMessageID()
{
    ++messageID_;
}
