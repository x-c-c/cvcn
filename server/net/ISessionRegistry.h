#pragma once
#include <vector>
#include <cstdint>

class IClientSession;

/**
 * @file ISessionRegistry.h
 * @brief Интерфейс индекса «userID → активная сессия».
 */
class ISessionRegistry
{
public:
    virtual ~ISessionRegistry() = default;

    virtual void registerUser(int userID, IClientSession* session) = 0;
    virtual void unregisterUser(int userID) = 0;
    virtual IClientSession* findByUserID(int userID) const = 0;
    virtual std::vector<IClientSession*> findByUserIDs(const std::vector<int>& userIDs) const = 0;
};
