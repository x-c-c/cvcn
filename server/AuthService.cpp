#include "AuthService.h"
#include "ClientSession.h"
#include "UserRepository.h"
#include "SessionRegistry.h"
#include "PacketBuilder.h"
#include "Logger.h"

AuthService::AuthService(UserRepository* userRepo, SessionRegistry* sessionRegistry): userRepo_(userRepo),sessionRegistry_(sessionRegistry){}

void AuthService::handleRegisterRequest(const PacketHeaderRaw& header,
										const RegisterRequestData& data,
										ClientSession& session)
{
	RegisterResponseData resp;
	if (userRepo_->isUserExist(data.username))
	{
		resp.success = 0;
		Logger::instance().warn("Register failed for '{}' (fd {}): user already exists",
			data.username, session.getfileDescriptor());
	}
	else
	{
		const std::string hash = "hash_" + data.password;
		resp.success = userRepo_->addUser(data.username, hash) ? 1 : 0;
		if (resp.success)
			Logger::instance().info("Register OK for '{}' (fd {})",
				data.username, session.getfileDescriptor());
		else
			Logger::instance().error("Register failed for '{}' (fd {}): DB error",
				data.username, session.getfileDescriptor());
	}
	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}

void AuthService::handleAuthRequest(const PacketHeaderRaw& header,
									const AuthRequestData& data,
									ClientSession& session)
{
	AuthResponseData resp;
	const std::string storedHash = userRepo_->getUserPasswordHash(data.username);
	resp.success = (!storedHash.empty() && storedHash == "hash_" + data.password) ? 1 : 0;

	if (resp.success)
	{
		const int userID = userRepo_->getUserID(data.username);
		session.setAuthenticated(userID, data.username);
		if (sessionRegistry_)
			sessionRegistry_->registerUser(userID, &session);

		Logger::instance().info("Auth OK for '{}' (fd {}, uid {})",
			data.username, session.getfileDescriptor(), userID);
	}
	else
	{
		Logger::instance().warn("Auth failed for '{}' (fd {})",
			data.username, session.getfileDescriptor());
	}
	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}

void AuthService::handleDeleteRequest(const PacketHeaderRaw& header,
									  const DeleteRequestData& data,
									  ClientSession& session)
{
	DeleteResponseData resp;
	const std::string hash = "hash_" + data.password;
	resp.success = userRepo_->deleteUser(data.username, hash) ? 1 : 0;

	if (resp.success)
		Logger::instance().info("Delete OK for '{}' (fd {})",
			data.username, session.getfileDescriptor());
	else
		Logger::instance().warn("Delete failed for '{}' (fd {}): user not found or wrong password",
			data.username, session.getfileDescriptor());

	session.sendRaw(PacketBuilder::buildPacket(header.messageID, header.sessionID, resp));
}
