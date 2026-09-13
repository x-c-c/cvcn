#include "SessionRegistry.h"
#include "IClientSession.h"

void SessionRegistry::registerUser(int userID, IClientSession* session)
{
    if (userID <= 0 || session == nullptr)
        return;
    byUserID_[userID] = session;
}

void SessionRegistry::unregisterUser(int userID)
{
    byUserID_.erase(userID);
}

IClientSession* SessionRegistry::findByUserID(int userID) const
{
    auto it = byUserID_.find(userID);
    return (it == byUserID_.end()) ? nullptr : it->second;
}

std::vector<IClientSession*> SessionRegistry::findByUserIDs(const std::vector<int>& userIDs) const
{
    std::vector<IClientSession*> result;
    result.reserve(userIDs.size());
    for (int userID : userIDs)
    {
        auto it = byUserID_.find(userID);
        if (it != byUserID_.end())
            result.push_back(it->second);
    }
    return result;
}
