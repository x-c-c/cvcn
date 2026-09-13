#pragma once
#include <cstdint>
#include "PacketData.h"

class IClientSession;
class IUserRepository;
class ISessionRegistry;

class AuthService
{
public:
    AuthService(IUserRepository* userRepo, ISessionRegistry* sessionRegistry);

    void handleRegisterRequest(const PacketHeaderRaw& header,
                               const RegisterRequestData& data,
                               IClientSession* session);

    void handleAuthRequest(const PacketHeaderRaw& header,
                           const AuthRequestData& data,
                           IClientSession* session);

    void handleDeleteRequest(const PacketHeaderRaw& header,
                             const DeleteRequestData& data,
                             IClientSession* session);

private:
    IUserRepository* userRepository_;
    ISessionRegistry* sessionRegistry_;
};
