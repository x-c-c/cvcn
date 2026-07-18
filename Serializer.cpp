#include "Serializer.h"

static void Serializer::appendBytes(std::vector<uint8_t>& dest, const void* src, size_t count)
{
	const size_t offset = dest.size();
	dest.resize(offset + count);
	std::memcpy(dest.data() + offset, src, count);
}
void Serializer::writeUint16(std::vector<uint8_t>& buf, uint16_t val)
{
	const uint16_t net = htons(val);
	appendBytes(buf, &net, sizeof(net));
}
void Serializer::writeUint32(std::vector<uint8_t>& buf, uint32_t val)
{
	const uint32_t net = htonl(val);
	appendBytes(buf, &net, sizeof(net));
}
void Serializer::writeString(std::vector<uint8_t>& buf, const std::string& str)
{
	const uint16_t len = static_cast<uint16_t>(str.size());
	writeUint16(buf, len);
	appendBytes(buf, str.data(), len);
}



/*====================================================================*/
/*							Connect Request							  */
/*====================================================================*/
std::vector<uint8_t> Serializer::buildConnectRequestPacket(uint32_t messageID, uint32_t sessionID)
{
	PacketHeaderRaw header;
	header.type_		= htons(static_cast<uint16_t>(PacketType::ConnectRequest));
	header.messageID_	= htonl(messageID);
	header.sessionID_	= htonl(sessionID);
	header.messageLen_	= 0;
	std::vector<uint8_t> result;
	appendBytes(result, &header, sizeof(header));
	return result;
}
/*====================================================================*/
/*							Connect Response			 			  */
/*====================================================================*/
std::vector<uint8_t> Serializer::buildConnectResponsePacket(uint32_t messageID, uint32_t sessionID)
{
	PacketHeaderRaw header;
	header.type_		= htons(static_cast<uint16_t>(PacketType::ConnectResponse));
	header.messageID_	= htonl(messageID);
	header.sessionID_	= htonl(sessionID);
	header.messageLen_	= 0;
	std::vector<uint8_t> result;
	appendBytes(result, &header, sizeof(header));
	return result;
}
/*====================================================================*/
/*							Register Request 						  */
/*====================================================================*/
std::vector<uint8_t> Serializer::buildRegisterRequestPacket(uint32_t messageID, uint32_t sessionID, const RegisterRequestPacket& packet)
{
	std::vector<uint8_t> body;
	writeString(body, packet.username);
	writeString(body, packet.password);
	
	PacketHeaderRaw header;
	header.type_		= htons(static_cast<uint16_t>(PacketType::RegisterRequest));
	header.messageID_	= htonl(messageID);
	header.sessionID_	= htonl(sessionID);
	header.messageLen_	= htons(static_cast<uint16_t>(body.size()));
	std::vector<uint8_t> result;
	appendBytes(result, &header, sizeof(header));
	appendBytes(result, body.data, body.size());
	
	return result;
}
/*====================================================================*/
/*							Register Response 						  */
/*====================================================================*/
std::vector<uint8_t> Serializer::buildRegisterResponsePacket(uint32_t messageID, uint32_t sessionID, const RegisterResponsePacket& packet)
{	
	std::vector<uint8_t> body;
	body.push_back(packet.success);
		
	PacketHeaderRaw header;
	header.type_		= htons(static_cast<uint16_t>(PacketType::RegisterResponse));
	header.messageID_	= htonl(messageID);
	header.sessionID_	= htonl(sessionID);
	header.messageLen_	= htons(static_cast<uint16_t>(body.size()));
	std::vector<uint8_t> result;
	appendBytes(result, &header, sizeof(header));
	appendBytes(result, body.data, body.size());
	
	return result;
}
/*====================================================================*/
/*							Auth Request 							  */
/*====================================================================*/
std::vector<uint8_t> Serializer::buildAuthRequestPacket(uint32_t messageID, uint32_t sessionID, const AuthRequestPacket& packet)
{
	std::vector<uint8_t> body;
	writeString(body, packet.username);
	writeString(body, packet.password);
	
	PacketHeaderRaw header;
	header.type_		= htons(static_cast<uint16_t>(PacketType::RegisterRequest));
	header.messageID_	= htonl(messageID);
	header.sessionID_	= htonl(sessionID);
	header.messageLen_	= htons(static_cast<uint16_t>(body.size()));
	std::vector<uint8_t> result;
	appendBytes(result, &header, sizeof(header));
	appendBytes(result, body.data, body.size());
	
	return result;
}
/*====================================================================*/
/*							Auth Response	 						  */
/*====================================================================*/
std::vector<uint8_t> Serializer::buildAuthResponsePacket(uint32_t messageID, uint32_t sessionID, const AuthResponsePacket& packet)
{	
	std::vector<uint8_t> body;
	body.push_back(packet.success);
		
	PacketHeaderRaw header;
	header.type_		= htons(static_cast<uint16_t>(PacketType::RegisterResponse));
	header.messageID_	= htonl(messageID);
	header.sessionID_	= htonl(sessionID);
	header.messageLen_	= htons(static_cast<uint16_t>(body.size()));
	std::vector<uint8_t> result;
	appendBytes(result, &header, sizeof(header));
	appendBytes(result, body.data, body.size());
	
	return result;
}
/*====================================================================*/
/*							Message Send 							  */
/*====================================================================*/
std::vector<uint8_t> Serializer::buildMessageSendPacket(uint32_t messageID, uint32_t sessionID, const MessageSendPacket& packet)
{	
	std::vector<uint8_t> body;
	writeUint32(body, packet.senderId);
	writeUint32(body, packet.chatId);
	writeString(body, packet.text);
		
	PacketHeaderRaw header;
	header.type_		= htons(static_cast<uint16_t>(PacketType::MessageSend));
	header.messageID_	= htonl(messageID);
	header.sessionID_	= htonl(sessionID);
	header.messageLen_	= htons(static_cast<uint16_t>(body.size()));
	std::vector<uint8_t> result;
	appendBytes(result, &header, sizeof(header));
	appendBytes(result, body.data, body.size());
	
	return result;
}
/*====================================================================*/
/*							Disconnect Request						  */
/*====================================================================*/
std::vector<uint8_t> Serializer::buildDisconnectPacket(uint32_t messageID, uint32_t sessionID, const DisconnectRequestPacket& packet)
{	
	PacketHeaderRaw header;
	header.type_		= htons(static_cast<uint16_t>(PacketType::MessageSend));
	header.messageID_	= htonl(messageID);
	header.sessionID_	= htonl(sessionID);
	header.messageLen_	= htons(static_cast<uint16_t>(body.size()));
	std::vector<uint8_t> result;
	appendBytes(result, &header, sizeof(header));
	appendBytes(result, body.data, body.size());
	
	return result;
}











