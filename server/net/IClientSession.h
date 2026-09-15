#pragma once
#include <string>
#include <vector>
#include <cstdint>

/**
 * @file IClientSession.h
 * @brief Интерфейс сессии, используемый сервисами.
 *
 * Все изменяющие методы (sendRaw, requestClose, setAuthenticated)
 * могут вызываться из любого потока. sendRaw/requestClose
 * асинхронные — они только кладут команду в ResultQueue.
 */
class IClientSession
{
public:
    virtual ~IClientSession() = default;

    virtual void sendRaw(const std::vector<uint8_t>& data) = 0;
    virtual void requestClose() = 0;
    virtual int getFileDescriptor() const = 0;
    virtual int getUserID() const = 0;
    virtual const std::string& getUsername() const = 0;
    virtual void setAuthenticated(int userID, const std::string& username) = 0;
    virtual bool isClosed() const = 0;
};
