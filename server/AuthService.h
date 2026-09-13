#pragma once
#include <cstdint>
#include "PacketData.h"

class ClientSession;
class UserRepository;
class SessionRegistry;

class AuthService
{
public:
	AuthService(UserRepository* userRepo, SessionRegistry* sessionRegistry);

	void handleRegisterRequest(const PacketHeaderRaw& header,
							   const RegisterRequestData& data,
							   ClientSession& session);

	void handleAuthRequest(const PacketHeaderRaw& header,
						   const AuthRequestData& data,
						   ClientSession& session);

	void handleDeleteRequest(const PacketHeaderRaw& header,
							 const DeleteRequestData& data,
							 ClientSession& session);

private:
	UserRepository* userRepo_;
	SessionRegistry* sessionRegistry_;
};
