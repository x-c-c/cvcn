#ifndef VALIDATOR_H
#define VALIDATOR_H

#pragma once
#include <string>
#include <cstddef>
#include <cstdint>
#include "AppConfig.h"

/**
 * @file Validator.h
 * @brief Валидация входящих данных.
 *
 * Используется и на клиенте, перед отправкой, и на сервере, перед обработкой.
 * Правила и лимиты берутся из config/AppConfig.h, чтобы клиент и сервер
 * не разошлись.
 */
class Validator
{
public:
    static constexpr size_t MAX_USERNAME_LENGTH = config::MAX_USERNAME_LENGTH;
    static constexpr size_t MAX_PASSWORD_LENGTH = config::MAX_PASSWORD_LENGTH;
    static constexpr size_t MAX_MESSAGE_LENGTH  = config::MAX_MESSAGE_LENGTH;

    static bool validateUsername(const std::string& username);
    static bool validatePassword(const std::string& password);
    static bool validateMessage(const std::string& message);
    static bool validateChatID(uint32_t chatID);

private:
    static bool isCredentialChar(char symbol);
    static bool isMessageChar(char symbol);
};

#endif // VALIDATOR_H
