#pragma once
#include <string>

/**
 * @file IUserRepository.h
 * @brief Интерфейс доступа к таблице users.
 */
class IUserRepository
{
public:
    virtual ~IUserRepository() = default;

    virtual bool userExists(const std::string& username) = 0;
    virtual bool addUser(const std::string& username, const std::string& passwordHash) = 0;
    virtual bool deleteUser(const std::string& username, const std::string& passwordHash) = 0;
    virtual std::string getUserPasswordHash(const std::string& username) = 0;
    virtual int getUserID(const std::string& username) = 0;
};
