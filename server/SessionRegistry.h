#pragma once
#include <unordered_map>
#include <vector>
#include <cstdint>

class ClientSession;

class SessionRegistry
{
public:
	void registerUser(int userID, ClientSession* session);
	void unregisterUser(int userID);

	ClientSession* findByUserID(int userID) const;
	std::vector<ClientSession*> findByUserIDs(const std::vector<int>& userIDs) const;

private:
	std::unordered_map<int, ClientSession*> byUserID_;
};
