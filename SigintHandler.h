#pragma once
#include <csignal>

class SigintHandler
{
private:
	static void handler(int signum);
	static volatile sig_atomic_t stopRequested_ = 0;
	
public:
	static void setup();
	static bool isStopRequested();
};
