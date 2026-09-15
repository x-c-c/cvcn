/**
 * @file    ShutdownSignal.h
 * @brief   Приём SIGINT/SIGTERM и единый флаг остановки сервера.
 *
 * @details
 *   Класс только ставит обработчики сигналов и предоставляет
 *   потокобезопасный «флаг остановки». Никакой логики завершения
 *   здесь нет: EventPoller и PortSelector сами периодически вызывают
 *   isStopRequested() и корректно выходят из своих циклов.
 *
 *   Обработчик сигнала не пишет в лог и не бросает исключений —
 *   внутри signal handler разрешено только выставлять
 *   sig_atomic_t. Всё остальное делается в основном коде, когда
 *   цикл заметит флаг.
 *
 * @note    Обработчики ставятся один раз в main до создания epoll-цикла.
 *          Все методы статические.
 * @warning Не использовать в многопоточном режиме без пересмотра:
 *          сейчас предполагается один основной поток.
 * @see     EventPoller, PortSelector
 */
#pragma once
#include <csignal>

class ShutdownSignal
{
private:
    /**
     * @brief Обработчик сигнала. Вызывается ядром асинхронно.
     * @param signum номер полученного сигнала (SIGINT или SIGTERM).
     */
	static void handler(int signum);
	
	/**
     * @brief Флаг «запрошена остановка». 0 — работать, 1 — остановиться.
     *
     * @details
     *   volatile — чтобы компилятор не закешировал значение в регистре
     *   и перечитывал его из памяти при каждой проверке.
     */
	static volatile sig_atomic_t stopRequested_;
	
public:
    /**
     * @brief Устанавливает обработчики SIGINT и SIGTERM.
     *
     * @details
     *   sa_flags = 0 — без SA_RESTART, чтобы системные вызовы (read,
     *   epoll_wait, getline) прерывались сигналом, а не перезапускались
     *   ядром. Это позволяет циклам быстрее заметить флаг остановки.
     *
     * @note Вызывается один раз в main до старта epoll-цикла.
     */
	static void setup();
	
	/**
     * @brief Проверяет, запрошена ли остановка.
     *
     * @return true — пришёл SIGINT/SIGTERM, пора выходить;
     *         false — работаем дальше.
     */
	static bool isStopRequested();
};
