#pragma once
#include <unordered_map>
#include <vector>
#include <cstdint>
#include "ISessionRegistry.h"

class IClientSession;

/**
 * @file SessionRegistry.h
 * @brief Индекс «userID → активная сессия».
 *
 * Не владеет сессиями. Только хранит указатели, которые
 * живут в SessionManager.
 */
class SessionRegistry : public ISessionRegistry
{
public:
    void registerUser(int userID, IClientSession* session) override;
    void unregisterUser(int userID) override;
    IClientSession* findByUserID(int userID) const override;
    std::vector<IClientSession*> findByUserIDs(const std::vector<int>& userIDs) const override;

private:
    std::unordered_map<int, IClientSession*> byUserID_;
};
