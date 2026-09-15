#include "ClientSession.h"
#include "EventPoller.h"
#include "PacketDispatcher.h"
#include "ResultQueue.h"
#include "AppConfig.h"
#include "Logger.h"
#include <cstring>
#include <cerrno>
#include <new>
#include <stdexcept>
#include <unistd.h>

ClientSession::ClientSession(int fileDescriptor,
                             EventPoller* eventPoller,
                             PacketDispatcher* dispatcher,
                             ResultQueue* resultQueue):
    fileDescriptor_(fileDescriptor),
    eventPoller_(eventPoller),
    dispatcher_(dispatcher),
    resultQueue_(resultQueue),
    sender_(eventPoller, fileDescriptor){}

ClientSession::~ClientSession()
{
    if (!closed_.load())
        closeSession();
}

std::vector<Task> ClientSession::handleRead()
{
    std::vector<Task> tasks;

    try
    {
        uint8_t tempBuffer[config::SESSION_READ_BUFFER_SIZE];
        ssize_t bytesRead = recv(fileDescriptor_, tempBuffer, sizeof(tempBuffer), 0);
        if (bytesRead > 0)
        {
            assembler_.appendData(tempBuffer, bytesRead);
            PacketHeaderRaw header;
            std::vector<uint8_t> body;
            while (assembler_.extractPacket(header, body))
            {
                if (header.messageLen > config::MAX_REASONABLE_PACKET_BODY)
                {
                    LOG_WARN("Packet too large: {} bytes (fd {})",
                        header.messageLen, fileDescriptor_);
                    closeSession();
                    return tasks;
                }
                tasks.push_back(Task{ this, header, std::move(body) });
            }
        }
        else if (bytesRead == 0)
        {
            LOG_INFO("Client {} closed connection", fileDescriptor_);
            closeSession();
        }
        else if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            LOG_ERROR("recv error on fd {}: {}", fileDescriptor_, strerror(errno));
            closeSession();
        }
    }
    catch (const std::bad_alloc&)
    {
        LOG_ERROR("Out of memory in handleRead (fd {})", fileDescriptor_);
        closeSession();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception in handleRead (fd {}): {}", fileDescriptor_, e.what());
        closeSession();
    }
    catch (...)
    {
        LOG_ERROR("Unknown exception in handleRead (fd {})", fileDescriptor_);
        closeSession();
    }

    return tasks;
}

void ClientSession::handleWrite()
{
    sender_.handleWrite();
}

void ClientSession::sendRaw(const std::vector<uint8_t>& data)
{
    if (!resultQueue_)
        return;

    SessionCommand cmd;
    cmd.type = CommandType::SendRaw;
    cmd.fd = fileDescriptor_;
    cmd.data = data;
    resultQueue_->push(std::move(cmd));
}

void ClientSession::requestClose()
{
    if (!resultQueue_)
        return;

    SessionCommand cmd;
    cmd.type = CommandType::Close;
    cmd.fd = fileDescriptor_;
    resultQueue_->push(std::move(cmd));
}

void ClientSession::sendRawDirect(const std::vector<uint8_t>& data)
{
    sender_.sendPacket(data);
}

void ClientSession::setAuthenticated(int userID, const std::string& username)
{
    userID_ = userID;
    username_ = username;
}

void ClientSession::closeSession()
{
    if (closed_.exchange(true))
        return;

    if (eventPoller_)
        eventPoller_->removeFileDescriptor(fileDescriptor_);
    close(fileDescriptor_);
    LOG_INFO("Session closed for fd {}", fileDescriptor_);
}
