#include "AuthService.h"
#include "IClientSession.h"
#include "IUserRepository.h"
#include "ISessionRegistry.h"
#include "PacketBuilder.h"
#include "AppConfig.h"
#include "Logger.h"

AuthService::AuthService(IUserRepository* userRepo, ISessionRegistry* sessionRegistry):
    userRepository_(userRepo),
    sessionRegistry_(sessionRegistry){}

void AuthService::handleRegisterRequest(const PacketHeaderRaw& header,
                                        const RegisterRequestData& data,
                                        IClientSession* session)
{
    RegisterResponseData resp;
    if (userRepository_->userExists(data.username))
    {
        resp.success = false;
        LOG_WARN("Register failed for '{}' (fd {}): user already exists",
            data.username, session->getFileDescriptor());
    }
    else
    {
        const std::string hash = std::string(config::PASSWORD_HASH_PREFIX) + data.password;
        resp.success = userRepository_->addUser(data.username, hash);
        if (resp.success)
            LOG_INFO("Register OK for '{}' (fd {})",
                data.username, session->getFileDescriptor());
        else
            LOG_ERROR("Register failed for '{}' (fd {}): DB error",
                data.username, session->getFileDescriptor());
    }
    session->sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}

void AuthService::handleAuthRequest(const PacketHeaderRaw& header,
                                    const AuthRequestData& data,
                                    IClientSession* session)
{
    AuthResponseData resp;
    const std::string storedHash = userRepository_->getUserPasswordHash(data.username);
    resp.success = (!storedHash.empty() &&
                    storedHash == std::string(config::PASSWORD_HASH_PREFIX) + data.password);

    if (resp.success)
    {
        const int userID = userRepository_->getUserID(data.username);
        session->setAuthenticated(userID, data.username);
        if (sessionRegistry_)
            sessionRegistry_->registerUser(userID, session);

        LOG_INFO("Auth OK for '{}' (fd {}, userID {})",
            data.username, session->getFileDescriptor(), userID);
    }
    else
    {
        LOG_WARN("Auth failed for '{}' (fd {})",
            data.username, session->getFileDescriptor());
    }
    session->sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}

void AuthService::handleDeleteRequest(const PacketHeaderRaw& header,
                                      const DeleteRequestData& data,
                                      IClientSession* session)
{
    DeleteResponseData resp;
    const std::string hash = std::string(config::PASSWORD_HASH_PREFIX) + data.password;
    resp.success = userRepository_->deleteUser(data.username, hash);

    if (resp.success)
        LOG_INFO("Delete OK for '{}' (fd {})",
            data.username, session->getFileDescriptor());
    else
        LOG_WARN("Delete failed for '{}' (fd {}): user not found or wrong password",
            data.username, session->getFileDescriptor());

    session->sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}
