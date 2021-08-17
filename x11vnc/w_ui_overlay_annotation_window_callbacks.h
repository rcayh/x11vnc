

#ifndef W_UI_OVERLAY_ANNOTATION_WINDOW_CALLBACKS_H_
#define W_UI_OVERLAY_ANNOTATION_WINDOW_CALLBACKS_H_


#include <gtk/gtk.h>

/* Expose event: this occurs when the windows is show .*/
G_MODULE_EXPORT gboolean
on_expose                    (GtkWidget *widget,
                              cairo_t   *cr,
                              gpointer   func_data);

/* On screen changed. */
void on_screen_changed       (GtkWidget *widget,
                              GdkScreen *previous_screen,
                              gpointer   user_data);


/* This is called when the button is pushed. */
G_MODULE_EXPORT gboolean
on_button_press              (GtkWidget      *win,
                              GdkEventButton *ev,
                              gpointer        func_data);


/* This shots when the pointer is moving. */
G_MODULE_EXPORT gboolean
on_motion_notify             (GtkWidget      *win,
                              GdkEventMotion *ev,
                              gpointer        func_data);


/* This shots when the button is released. */
G_MODULE_EXPORT gboolean
on_button_release            (GtkWidget      *win,
                              GdkEventButton *ev,
                              gpointer        func_data);


/* On device added. */
void on_device_removed       (GdkDeviceManager *device_manager,
                              GdkDevice        *device,
                              gpointer          user_data);
			

/* On device removed. */
void on_device_added         (GdkDeviceManager *device_manager,
                              GdkDevice        *device,
                              gpointer          user_data);


#endif


