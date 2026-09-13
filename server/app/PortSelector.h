/**
 * @file PortSelector.h
 * @brief Утилиты для проверки и интерактивного выбора TCP‑порта.
 */

#pragma once

/**
 * @brief Проверяет, свободен ли TCP‑порт.
 * @param port номер порта (1...65535)
 * @return true если порт свободен, false если занят или произошла ошибка
 */
bool isPortFree(int port);

/**
 * @brief Интерактивно запрашивает у пользователя свободный порт.
 * @param defaultPort порт, предлагаемый по умолчанию
 * @return гарантированно свободный порт или возвращает -1 при запросе остановки
 */
int promptForPort(int defaultPort);
