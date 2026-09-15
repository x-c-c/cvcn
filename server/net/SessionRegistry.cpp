#include "SessionRegistry.h"
#include "IClientSession.h"

void SessionRegistry::registerUser(int userID, IClientSession* session)
{
    if (userID <= 0 || session == nullptr)
        return;
    std::lock_guard<std::mutex> lock(mutex_);
    byUserID_[userID] = session;
}

void SessionRegistry::unregisterUser(int userID)
{
    std::lock_guard<std::mutex> lock(mutex_);
    byUserID_.erase(userID);
}

IClientSession* SessionRegistry::findByUserID(int userID) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = byUserID_.find(userID);
    return (it == byUserID_.end()) ? nullptr : it->second;
}

std::vector<IClientSession*> SessionRegistry::findByUserIDs(const std::vector<int>& userIDs) const
{
    std::vector<IClientSession*> result;
    result.reserve(userIDs.size());
    std::lock_guard<std::mutex> lock(mutex_);
    for (int userID : userIDs)
    {
        auto it = byUserID_.find(userID);
        if (it != byUserID_.end())
            result.push_back(it->second);
    }
    return result;
}
