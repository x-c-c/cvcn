#pragma once

/**
 * @file AppState.h
 * @brief Состояние клиентского приложения.
 *
 *   Disconnected    — соединение не установлено (старт или после разрыва)
 *   Connected       — TCP установлен, пользователь ещё не вошёл
 *   Authenticating  — запрос авторизации отправлен, ответа нет
 *   Authenticated   — пользователь успешно вошёл
 */
enum class AppState
{
    Disconnected,
    Connected,
    Authenticating,
    Authenticated
};
