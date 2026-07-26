#include "SigintHandler.h"

void SigintHandler::setup()
{
	std::signal(SIGINT, handler);
	std::signal(SIGTERM, handler);
}

void SigintHandler::handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
		stopRequested_ = true;
}

bool SigintHandler::isStopRequested()
{
	return stopRequested_ != 0;
}
