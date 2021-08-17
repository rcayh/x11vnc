
#ifndef W_UI_H_
#define W_UI_H_

#include "w_common.h"

MUTEX(uiMutex);

int 	ui_main(int argc, char* argv[]);
void* 	ui_time_loop(void* ptr);
void 	ui_set_status_text(const char* message);
void 	ui_finish();
void 	ui_resume();
void 	ui_connected();
void 	ui_chat_receive_message(const char* message);


#endif
