#ifndef VALIDATOR_H
#define VALIDATOR_H

#pragma once
#include <string>
#include <cstddef>
#include <cstdint>

/**
 * @brief Валидатор входящих данных.
 *
 * Используется и на клиенте, перед отправкой, и на сервере, перед обработкой, для безопасности
 * Правила одинаковы на обеих сторонах
 */
class Validator
{
public:
    // Ограничения длины
    static constexpr size_t MAX_USERNAME_LENGTH = 64;
    static constexpr size_t MAX_PASSWORD_LENGTH = 127;
    static constexpr size_t MAX_MESSAGE_LENGTH  = 4096;

    // Логин и пароль: латиница, цифры, - _ ! @ ^ ?
    static bool validateUsername(const std::string& username);
    static bool validatePassword(const std::string& password);

    // Текст сообщения: печатные ASCII + табуляция + перевод строки
    static bool validateMessage(const std::string& message);

    // Идентификаторы
    static bool validateSenderID(uint32_t senderID);
    static bool validateChatID(uint32_t chatID);

private:
    static bool isCredentialChar(char symbol);
    static bool isMessageChar(char symbol);
};

#endif // VALIDATOR_H
