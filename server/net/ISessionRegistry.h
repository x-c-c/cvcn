#pragma once
#include <vector>
#include <cstdint>

class IClientSession;

/**
 * @file ISessionRegistry.h
 * @brief Интерфейс индекса «userID → активная сессия».
 */
class ISessionRegistry
{
public:
    virtual ~ISessionRegistry() = default;

    /**
     * @brief Зарегистрировать сессию пользователя.
     *
     * @param userID  ID пользователя (> 0)
     * @param session указатель на живую сессию (не владеет)
     *
     * @pre  userID > 0 и session != nullptr. Иначе вызов игнорируется.
     * @post findByUserID(userID) возвращает session до вызова unregisterUser().
     * @note Перезаписывает предыдущую сессию того же пользователя.
     */

    virtual void registerUser(int userID, IClientSession* session) = 0;
    /**
     * @brief Убрать сессию пользователя из реестра.
     *
     * @param userID  ID пользователя
     * @param session указатель на ту сессию, которую нужно убрать
     *
     * @post Запись удаляется только если в реестре лежит именно session.
     *       Если пользователь успел войти заново и в реестре уже другая
     *       сессия — она не тронута.
     * @note Это защита от «вошёл дважды»: старая сессия при закрытии
     *       не должна выкинуть новую живую.
     */

    virtual void unregisterUser(int userID, IClientSession* session) = 0;
    /**
     * @brief Найти сессию по ID пользователя.
     * @param userID ID пользователя
     * @return указатель на сессию или nullptr, если не найдена.
     * @note Возвращённый указатель может стать висячим, если сессию
     *       закроют из main-thread. Использовать сразу.
     */

    virtual IClientSession* findByUserID(int userID) const = 0;
    /**
     * @brief Найти сессии по списку ID.
     * @param userIDs список ID пользователей
     * @return вектор найденных сессий; отсутствующие ID пропускаются.
     */

    virtual std::vector<IClientSession*> findByUserIDs(const std::vector<int>& userIDs) const = 0;
};
