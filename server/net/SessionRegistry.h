#pragma once
#include <unordered_map>
#include <vector>
#include <mutex>
#include <cstdint>
#include "ISessionRegistry.h"

class IClientSession;

/**
 * @file SessionRegistry.h
 * @brief Индекс «userID → активная сессия». Thread-safe.
 */
class SessionRegistry : public ISessionRegistry
{
public:
    void registerUser(int userID, IClientSession* session) override;
    void unregisterUser(int userID, IClientSession* session) override;
    IClientSession* findByUserID(int userID) const override;
    std::vector<IClientSession*> findByUserIDs(const std::vector<int>& userIDs) const override;

private:
    mutable std::mutex mutex_;
    std::unordered_map<int, IClientSession*> byUserID_;
};
