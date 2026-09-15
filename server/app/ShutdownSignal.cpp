/**
 * @file    ShutdownSignal.cpp
 * @brief   Реализация обработчиков сигналов и флага остановки.
 * @see ShutdownSignal.h
 */
#include "./ShutdownSignal.h"

volatile sig_atomic_t ShutdownSignal::stopRequested_ = 0;

void ShutdownSignal::setup()
{
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, nullptr);
	sigaction(SIGTERM, &sa, nullptr);
}

void ShutdownSignal::handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
		stopRequested_ = 1;
}
bool ShutdownSignal::isStopRequested()
{
	return stopRequested_ != 0;
}
