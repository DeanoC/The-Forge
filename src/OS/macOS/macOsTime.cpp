#ifdef __APPLE__
#include <ctime>
#include <cmath>

/************************************************************************/
// Time Related Functions
/************************************************************************/

unsigned getSystemTime()
{
	long            ms;    // Milliseconds
	time_t          s;     // Seconds
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);

	s = spec.tv_sec;
	ms = round(spec.tv_nsec / 1.0e6);    // Convert nanoseconds to milliseconds

	ms += s * 1000;

	return (unsigned int)ms;
}

int64_t getUSec()
{
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long us = (ts.tv_nsec / 1000);
	us += ts.tv_sec * 1e6;
	return us;
}

int64_t getTimerFrequency()
{
	return 1;
}

unsigned getTimeSinceStart() { return (unsigned)time(NULL); }


#endif