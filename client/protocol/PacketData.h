/**
 * @file    PacketData.h
 * @brief   Описание протокола: типы пакетов, заголовок и структуры тел.
 *
 * @details
 *   Единственный файл, который описывает «что такое пакет» на уровне
 *   данных. Файл — общий для клиента и сервера: одни и те же байты
 *   должны интерпретироваться одинаково с обеих сторон, поэтому
 *   любые изменения здесь обязаны быть синхронными в обоих
 *   каталогах (client/protocol и server/protocol).
 *
 *   Состав:
 *     - PacketType         — enum типов пакетов (поле header.type);
 *     - PacketHeaderRaw    — 12-байтовый заголовок, всегда первый;
 *     - *Data-структуры    — тела пакетов, по одной на тип.
 *
 *   Формат на проводе (все многобайтовые числа — big-endian):
 *     [type:2][messageID:4][sessionID:4][messageLen:2][body:messageLen]
 *
 *   Тело пакета сериализуется PacketBuilder-ом и разбирается
 *   PacketParser-ом. Сами структуры — просто контейнеры,
 *   никакой логики в них нет.
 *
 * @note    PacketHeaderRaw упакован #pragma pack(1) — иначе компилятор
 *          вставил бы padding между uint16_t и uint32_t, и sizeof
 *          стал бы 16 вместо 12, что сломало бы протокол.
 * @warning Менять порядок/типы полей можно только одновременно на
 *          клиенте и сервере. Иначе битые пакеты и непонятные ошибки.
 * @see     PacketBuilder, PacketParser, ByteReader, ByteWriter
 */

#pragma once
#include <cstdint>
#include <string>

/**
 * @brief Тип пакета. Хранится в первом поле заголовка.
 *
 * @details
 *   Значения 0x1..0x8 — рабочие.
 *
 * @note Тип uint16_t — для будущего расширения (запас 0x9..0xFFFF).
 */
enum class PacketType : uint16_t
{
    ConnectRequest      = 0x1,  ///< Запрос подключения (без тела).
    ConnectResponse     = 0x2,  ///< Ответ на подключение (без тела).
    RegisterRequest     = 0x3,  ///< Запрос регистрации.
    RegisterResponse    = 0x4,  ///< Ответ на регистрацию.
    AuthRequest         = 0x5,  ///< Запрос аутентификации.
    AuthResponse        = 0x6,  ///< Ответ на аутентификацию.
    MessageSend         = 0x7,  ///< Отправка текстового сообщения.
    DisconnectRequest   = 0x8   ///< Запрос отключения (без тела).
};

/**
 * @brief Заголовок любого пакета. Всегда 12 байт, всегда первый.
 *
 * @details
 *   Раскладка на проводе (network byte order):
 *     offset  size  field
 *        0      2    type
 *        2      4    messageID
 *        6      4    sessionID
 *       10      2    messageLen
 *
 *   В структуре поля лежат уже в host byte order — конвертация
 *   делается в PacketParser::deserializeHeader (ntohs/ntohl) и
 *   PacketBuilder::buildPacket (htons/htonl).
 *
 * @note    #pragma pack(1) обязателен: без него компилятор вставит
 *          2 байта padding-а перед messageID, и sizeof станет 16.
 *          memcpy в PacketParser тогда скопирует лишнее.
 * @warning Не использовать эту структуру для передачи по сети
 *          «как есть» без конвертации byte order. На little-endian
 *          машинах (x86) числа уйдут в обратном порядке.
 */
#pragma pack(push, 1)
struct PacketHeaderRaw
{
    uint16_t type;          ///< Тип пакета (значение PacketType).
    uint32_t messageID;     ///< Уникален в рамках сессии, растёт с каждым запросом.
    uint32_t sessionID;     ///< Идентификатор сессии. 0 — до ConnectRequest.
    uint16_t messageLen;    ///< Длина тела в байтах (после заголовка).
};
#pragma pack(pop)

//  Тела пакетов 
// Каждая структура — «полезная нагрузка» одного типа. Сериализуется
// PacketBuilder::buildPacket(uint32_t, uint32_t, const XxxData&) и
// разбирается PacketParser::deserializeData(body, XxxData&).
//
// Правило: поля идут в том порядке, в котором пишутся в тело.
// Пустые структуры ({}), соответствующие пакетам без тела,
// существуют только для перегрузки — реальных данных не несут.

/// @brief ConnectRequest: тело пустое, sessionID назначает сервер.
struct ConnectRequestData {};

/// @brief ConnectResponse: тело пустое, sessionID берётся из заголовка.
struct ConnectResponseData {};

/**
 * @brief RegisterRequest: логин и пароль нового пользователя.
 *
 * @note Сериализуется как две строки: [len:2][bytes] для каждой.
 *       Максимальная длина ограничена 65535 байт на строку (uint16_t
 *       в префиксе длины).
 * @warning Пароль пока передаётся в открытом виде. В будущем — хэш.
 */
struct RegisterRequestData
{
    std::string username;   ///< Имя пользователя (уникальное).
    std::string password;   ///< Пароль (в будущем — хэш).
};

/**
 * @brief RegisterResponse: результат попытки регистрации.
 */
struct RegisterResponseData
{
    uint8_t success;        ///< 1 — успех, 0 — ошибка (логин занят и т.п.).
};

/**
 * @brief AuthRequest: логин и пароль для входа.
 *
 * @note Сериализация идентична RegisterRequestData.
 * @warning Пароль пока в открытом виде.
 */
struct AuthRequestData
{
    std::string username;   ///< Имя пользователя.
    std::string password;   ///< Пароль (в будущем — хэш).
};

/**
 * @brief AuthResponse: результат аутентификации.
 */
struct AuthResponseData
{
    uint8_t success;        ///< 1 — успех, 0 — неверный логин/пароль.
};

/**
 * @brief MessageSend: отправка сообщения в чат.
 *
 * @note Порядок на проводе: senderID, chatID, text. senderID —
 *       идентификатор отправителя (сейчас передаётся клиентом,
 *       в будущем сервер подставит его сам из сессии).
 */
struct MessageSendData
{
    uint32_t senderID;      ///< Идентификатор отправителя.
    uint32_t chatID;        ///< Идентификатор чата/получателя.
    std::string text;       ///< Текст сообщения (UTF-8).
};

/// @brief DisconnectRequest: тело пустое, сервер закрывает сессию.
struct DisconnectRequestData {};
