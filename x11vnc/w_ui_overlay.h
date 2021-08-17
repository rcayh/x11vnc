
#ifndef W_UI_OVERLAY_H_
#define W_UI_OVERLAY_H_

#include "w_common.h"

MUTEX(drawMutex);

#define POINTER_ACTION_DOWN 0
#define POINTER_ACTION_UP 1
#define POINTER_ACTION_MOVE 2


#define PEN_TYPE_FREELINE 1
#define PEN_TYPE_LINE 2
#define PEN_TYPE_RECT 3
#define PEN_TYPE_CIRCLE 4
#define PEN_TYPE_ARROWS 5

typedef struct _RedpenInfo{
	unsigned char type;
	unsigned char onoff;
	unsigned short erase;
	unsigned short PenType;
	unsigned short width;
	unsigned long color;
	int downX;
	int downY;
	int windowPosX;
	int windowPosY;
} RedpenInfo, *RedpenInfoPtr;


typedef struct _PointerEvent{

	int action;
	int x;
	int y;
	int mask;

} PointerEvent;

void draw_test();
int overlay_create(RedpenInfo penInfo);
int overlay_destroy();
int overlay_pointer_event_message(PointerEvent evt);

#endif

