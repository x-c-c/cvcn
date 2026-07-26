#include "SigintHandler.h"

volatile sig_atomic_t SigintHandler::stopRequested_ = 0;

void SigintHandler::setup()
{
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;					// без SA_RESTART, чтобы getline не перезапускался автоматически
	sigaction(SIGINT, &sa, nullptr);
	sigaction(SIGTERM, &sa, nullptr);
}

void SigintHandler::handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
		stopRequested_ = 1;
}
bool SigintHandler::isStopRequested()
{
	return stopRequested_ != 0;
}
