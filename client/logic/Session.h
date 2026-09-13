#pragma once
#include <QString>
#include "AppState.h"

/**
 * @file Session.h
 * @brief Текущее состояние клиента: статус соединения и имя вошедшего пользователя.
 *
 * Хранит только данные. Не эмитит сигналов, не содержит логики переходов.
 * Управление состоянием — в AuthController и MainController.
 */
class Session
{
public:
    AppState state() const { return state_; }
    void setState(AppState state) { state_ = state; }

    const QString& username() const { return username_; }
    void setUsername(const QString& username) { username_ = username; }

    bool isAuthenticated() const { return state_ == AppState::Authenticated; }

private:
    AppState state_ = AppState::Disconnected;
    QString username_;
};
