
#include "w_common.h"

static wlogMutex_initialized = 0;

static void wDefaultLog(const char *format, ...){

#ifdef DIST_TYPE_RELEASE

#else
	va_list args;
	char buf[256];
	time_t log_clock;

	if(! wlogMutex_initialized) {
		INIT_MUTEX(wlogMutex);
		wlogMutex_initialized = 1;
	}

	LOCK(wlogMutex);
	va_start(args, format);

	time(&log_clock);
	strftime(buf, 255, "%Y-%m-%d %H:%M:%S - ", localtime(&log_clock));
	fprintf(stderr, "%s", buf);

	vfprintf(stderr, format, args);
	fflush(stderr);

	va_end(args);
	UNLOCK(wlogMutex);
#endif

}

wLogProc wLog = wDefaultLog;
