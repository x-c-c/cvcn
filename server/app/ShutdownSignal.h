#pragma once
#include <csignal>

class ShutdownSignal
{
private:
	static void handler(int signum);
	static volatile sig_atomic_t stopRequested_;
	
public:
	static void setup();
	static bool isRequested();
};
