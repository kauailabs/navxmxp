/*
 * Timer.h
 *
 *  Created on: 18 Jan 2017
 *      Author: pi
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <time.h>

class Timer
{
	static double TimeSpecToSeconds(struct timespec* ts)
	{
	    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
	}

public:
	static float GetMonotonicTimestamp() {
		struct timespec now;
		now.tv_sec = 0;
		now.tv_nsec = 0;
		double now_seconds;
		if(!clock_gettime(CLOCK_MONOTONIC, &now))
		{
			now_seconds = TimeSpecToSeconds(&now);
		}
		return now_seconds;
	}
};


#endif /* TIMER_H_ */
