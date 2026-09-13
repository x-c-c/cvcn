#pragma once
#include <string>
#include <sqlite3.h>
#include "IUserRepository.h"

class UserRepository : public IUserRepository
{
public:
    explicit UserRepository(sqlite3* db);

    bool userExists(const std::string& username) override;
    bool addUser(const std::string& username, const std::string& passwordHash) override;
    bool deleteUser(const std::string& username, const std::string& passwordHash) override;

    std::string getUserPasswordHash(const std::string& username) override;
    int getUserID(const std::string& username) override;

private:
    sqlite3* db_;
};
