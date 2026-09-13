#include "SessionRegistry.h"

void SessionRegistry::registerUser(int userID, ClientSession* session)
{
	if (userID <= 0 || session == nullptr)
		return;
	byUserID_[userID] = session;
}

void SessionRegistry::unregisterUser(int userID)
{
	byUserID_.erase(userID);
}

ClientSession* SessionRegistry::findByUserID(int userID) const
{
	auto it = byUserID_.find(userID);
	return (it == byUserID_.end()) ? nullptr : it->second;
}

std::vector<ClientSession*> SessionRegistry::findByUserIDs(const std::vector<int>& userIDs) const
{
	std::vector<ClientSession*> result;
	result.reserve(userIDs.size());
	for (int uid : userIDs)
	{
		auto it = byUserID_.find(uid);
		if (it != byUserID_.end())
			result.push_back(it->second);
	}
	return result;
}
