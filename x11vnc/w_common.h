#ifndef W_COMMON_H
#define W_COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>

#include "w_options.h"

#ifdef LIBVNCSERVER_GTK2
#define LIB_GTK2
#else
#define LIB_GTK3
#endif

#undef MUTEX
#define MUTEX(mutex) pthread_mutex_t (mutex)
#undef INIT_MUTEX
#define INIT_MUTEX(mutex) pthread_mutex_init(&(mutex),NULL)
#undef LOCK
#define LOCK(mutex) pthread_mutex_lock(&(mutex));
#undef UNLOCK
#define UNLOCK(mutex) pthread_mutex_unlock(&(mutex));

#undef bool
#define bool int
#undef true
#define true 1
#undef false
#define false 0

#define DHPRO
//#define HAVE_LIB_NOTIFY 1

extern bool dist_type_release;
#ifdef LIBVNCSERVER_DIST_TYPE_RELEASE
#define DEFAULT_HTTP_URI ""
#define DIST_TYPE_RELEASE
#else
#define DEFAULT_HTTP_URI ""
//#define REQUEST_TEST_CONTROL_LIST
#endif


#define DEVICE_TYPE_LINUX 4


#define REQ_RESULT_ACCEPTNUM_EMPTY -2

typedef enum _StatusNofity{
	STATUS_NOTIFY_NONE,
	STATUS_NOTIFY_BEGIN,
	STATUS_NOTIFY_REQ_ACCEPTNUM,
	STATUS_NOTIFY_CONNECT_CMD_SOCK,
	STATUS_NOTIFY_RESUME_REPLAY_SOCK,
	STATUS_NOTIFY_CONNECTED,
	STATUS_NOTIFY_DISCONNECTED,
} StatusNotify;

typedef struct _ControlWaitInfo{

	char acceptNum[10];
	char cpCode[20];
	char userId[30];
	char controlWaitSystemId[50];
	char mySystemId[50];
	char relayHost[50];
	int relayPort;
	char webServerHost[50];
	int webServerPort;
	char webServerScheme[10];
	bool success;
	int reqResult;

} ControlWaitInfo, *ControlWaitInfoPtr;


MUTEX(wlogMutex);
typedef void (*wLogProc)(const char *format, ...);
extern wLogProc wLog;
extern bool change_pixel_size;

void* start_main(void* ptr);
void send_cmd_destroy();


#endif
