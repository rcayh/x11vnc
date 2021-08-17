
#ifndef W_UI_OVERLAY_BACKGROUND_WINDOW_CALLBACKS_H_
#define W_UI_OVERLAY_BACKGROUND_WINDOW_CALLBACKS_H_


#include <gtk/gtk.h>

/* On screen changed. */
void on_back_screen_changed       (GtkWidget *widget,
                                   GdkScreen *previous_screen,
                                   gpointer   user_data);


#endif
