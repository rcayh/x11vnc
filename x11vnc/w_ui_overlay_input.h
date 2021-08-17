#ifndef W_UI_OVERLAY_INPUT_H_
#define W_UI_OVERLAY_INPUT_H_

#include "w_ui_overlay_annotation_window.h"
#include <glib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#ifdef _WIN32
#  include <cairo-win32.h>
#  include <gdkwin32.h>
#  include <winuser.h>
#else
#  ifdef __APPLE__
#    include <cairo-quartz.h>
#  else
#    include <cairo-xlib.h>
#  endif
#endif


/* Un-grab pointer. */
void
ungrab_pointer     (GdkDisplay         *display);


/* Grab pointer. */
void
grab_pointer       (GtkWidget            *win,
                    GdkEventMask          eventmask);


/* Remove all the devices . */
void
remove_input_devices    (AnnotateData *data);

/* Set-up input devices. */
void
setup_input_devices     (AnnotateData  *data);


/* Add input device. */
void
add_input_device   (GdkDevice        *device,
                    AnnotateData     *data);


/* Remove input device. */
void
remove_input_device     (GdkDevice     *device,
                         AnnotateData  *data);


#endif

