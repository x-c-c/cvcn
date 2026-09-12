#include "Validator.h"

bool Validator::isCredentialChar(char symbol)
{
    if (symbol >= 'a' && symbol <= 'z')
        return true;
    if (symbol >= 'A' && symbol <= 'Z')
        return true;
    if (symbol >= '0' && symbol <= '9')
        return true;
    switch (symbol)
    {
    case '-':
    case '_':
    case '!':
    case '@':
    case '^':
    case '?':
        return true;
    default:
        return false;
    }
}

bool Validator::isMessageChar(char symbol)
{
    // Разрешаем перевод строки
    if (symbol == '\n')
        return true;
    // Отклоняем управляющие символы и DEL
    if (static_cast<unsigned char>(symbol) < 0x20 || symbol == 0x7F)
        return false;
    // Остальные печатные ASCII и старшие байты (UTF-8) пропускаем
    return true;
}

bool Validator::validateUsername(const std::string& username)
{
    if (username.empty() || username.size() > MAX_USERNAME_LENGTH)
        return false;
    for (char symbol : username)
    {
        if (!isCredentialChar(symbol))
            return false;
    }
    return true;
}

bool Validator::validatePassword(const std::string& password)
{
    if (password.empty() || password.size() > MAX_PASSWORD_LENGTH)
        return false;
    for (char symbol : password)
    {
        if (!isCredentialChar(symbol))
            return false;
    }
    return true;
}

bool Validator::validateMessage(const std::string& message)
{
    if (message.empty() || message.size() > MAX_MESSAGE_LENGTH)
        return false;
    for (char symbol : message)
    {
        if (!isMessageChar(symbol))
            return false;
    }
    return true;
}

bool Validator::validateSenderID(uint32_t senderID)
{
    return senderID > 0;
}

bool Validator::validateChatID(uint32_t chatID)
{
    return chatID > 0;
}
