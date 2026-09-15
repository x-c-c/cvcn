#pragma once
#include <string>
#include <vector>
#include <cstdint>

/**
 * @file IClientSession.h
 * @brief Интерфейс сессии, используемый сервисами.
 *
 * Все изменяющие методы (sendRaw, requestClose, setAuthenticated)
 * могут вызываться из любого потока. sendRaw/requestClose
 * асинхронные — они только кладут команду в ResultQueue.
 */
class IClientSession
{
public:
    virtual ~IClientSession() = default;

    /**
     * @brief Поставить готовый пакет в очередь на отправку клиенту.
     *
     * @param data пакет целиком (заголовок + тело), big-endian
     *
     * @post Команда SendRaw оказалась в ResultQueue, main-thread
     *       получит уведомление через eventfd.
     * @note Не блокирует. Безопасно из любого потока.
     */

    virtual void sendRaw(const std::vector<uint8_t>& data) = 0;
    /**
     * @brief Запросить закрытие сессии.
     *
     * @post В ResultQueue лежит команда Close. Сессия будет закрыта
     *       main-thread'ом после завершения in-flight задач по fd.
     * @note Идемпотентно. Безопасно из любого потока.
     */

    virtual void requestClose() = 0;
    /**
     * @brief Получить файловый дескриптор сокета.
     * @return fd >= 0, либо -1 если сессия уже уничтожена.
     */

    virtual int getFileDescriptor() const = 0;
    /**
     * @brief ID аутентифицированного пользователя.
     * @return ID > 0 после успешного Auth, иначе -1.
     */

    virtual int getUserID() const = 0;
    /**
     * @brief Имя аутентифицированного пользователя.
     * @return Пустая строка, если пользователь ещё не вошёл.
     */

    virtual const std::string& getUsername() const = 0;
    /**
     * @brief Пометить сессию как аутентифицированную.
     *
     * @param userID   ID пользователя (должен быть > 0)
     * @param username имя пользователя (непустое)
     *
     * @pre  Вызывается после успешной проверки пароля в AuthService.
     * @post getUserID() > 0 и getUsername() == username.
     * @note Вызывается из воркера до регистрации в SessionRegistry,
     *       поэтому другие потоки не увидят частично заполненные поля.
     */

    virtual void setAuthenticated(int userID, const std::string& username) = 0;
    /**
     * @brief Проверить, закрыта ли сессия.
     * @return true, если closeSession() уже отработал.
     * @note Атомарное чтение, безопасно из любого потока.
     */

    virtual bool isClosed() const = 0;
};
