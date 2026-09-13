#include "AuthService.h"
#include "ClientSession.h"
#include "UserRepository.h"
#include "SessionRegistry.h"
#include "PacketBuilder.h"
#include "Logger.h"

AuthService::AuthService(UserRepository* userRepo, SessionRegistry* sessionRegistry): userRepository_(userRepo),sessionRegistry_(sessionRegistry){}

void AuthService::handleRegisterRequest(const PacketHeaderRaw& header,
										const RegisterRequestData& data,
										ClientSession& session)
{
	RegisterResponseData resp;
	if (userRepository_->userExists(data.username))
	{
		resp.success = false;
		LOG_WARN("Register failed for '{}' (fd {}): user already exists",
			data.username, session.getFileDescriptor());
	}
	else
	{
		const std::string hash = "hash_" + data.password;
		resp.success = userRepository_->addUser(data.username, hash);
		if (resp.success)
			LOG_INFO("Register OK for '{}' (fd {})",
				data.username, session.getFileDescriptor());
		else
			LOG_ERROR("Register failed for '{}' (fd {}): DB error",
				data.username, session.getFileDescriptor());
	}
	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}

void AuthService::handleAuthRequest(const PacketHeaderRaw& header,
									const AuthRequestData& data,
									ClientSession& session)
{
	AuthResponseData resp;
	const std::string storedHash = userRepository_->getUserPasswordHash(data.username);
	resp.success = (!storedHash.empty() && storedHash == "hash_" + data.password);

	if (resp.success)
	{
		const int userID = userRepository_->getUserID(data.username);
		session.setAuthenticated(userID, data.username);
		if (sessionRegistry_)
			sessionRegistry_->registerUser(userID, &session);

		LOG_INFO("Auth OK for '{}' (fd {}, userID {})",
			data.username, session.getFileDescriptor(), userID);
	}
	else
	{
		LOG_WARN("Auth failed for '{}' (fd {})",
			data.username, session.getFileDescriptor());
	}
	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}

void AuthService::handleDeleteRequest(const PacketHeaderRaw& header,
									  const DeleteRequestData& data,
									  ClientSession& session)
{
	DeleteResponseData resp;
	const std::string hash = "hash_" + data.password;
	resp.success = userRepository_->deleteUser(data.username, hash);

	if (resp.success)
		LOG_INFO("Delete OK for '{}' (fd {})",
			data.username, session.getFileDescriptor());
	else
		LOG_WARN("Delete failed for '{}' (fd {}): user not found or wrong password",
			data.username, session.getFileDescriptor());

	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}
