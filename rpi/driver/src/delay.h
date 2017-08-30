#include <iostream>
#include <chrono>
#include <thread>
#include <time.h> /* nanosleep() */

inline void delayMillis(int ms) {
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = ms * 1000000;
	nanosleep(&ts, NULL);
}
