
#ifndef W_UI_OVERLAY_BAR_H_
#define W_UI_OVERLAY_BAR_H_

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "w_ui_overlay.h"


/* Distance space from border to the ardesia bar in pixel unit. */
#define SPACE_FROM_BORDER 35

#define MICRO_THICKNESS   3
#define THIN_THICKNESS    6
#define MEDIUM_THICKNESS  12
#define THICK_THICKNESS   18    


/* Structure that contains the info passed to the callbacks. */
typedef struct
{

  /* rectifier flag. */
  gboolean rectifier;

  /* rounder flag. */
  gboolean rounder;

  /* selected colour in RGBA format. */
  gchar* color;

  /* selected line thickness. */
  gint thickness;

  /* annotation is visible. */
  gboolean annotation_is_visible;

  /* grab when leave. */
  gboolean grab;


  GtkWidget *bar_window;

  GtkBuilder *bar_window_gtk_builder;


}BarData;


/* Create the ardesia bar window. */

void destroy_bar_window();


GtkWidget *
create_bar_window (
                   GtkWidget   *parent);



#define BAR_UI_GLADE_TEXT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"								\
"<interface>"																						\
"<object class=\"GtkWindow\" id=\"ArdesiaBar\">"													\
    "<property name=\"width_request\">0</property>"													\
    "<property name=\"height_request\">0</property>"												\
    "<property name=\"can_focus\">True</property>"													\
    "<property name=\"has_focus\">True</property>"													\
	"<property name=\"is_focus\">True</property>"													\
    "<property name=\"events\">GDK_LEAVE_NOTIFY_MASK</property>"									\
	"<property name=\"title\">d window</property>"													\
    "<property name=\"resizable\">False</property>"													\
    "<property name=\"destroy_with_parent\">True</property>"										\
    "<property name=\"skip_taskbar_hint\">True</property>"											\
    "<property name=\"skip_pager_hint\">True</property>"											\
    "<property name=\"decorated\">False</property>"													\
    "<property name=\"gravity\">center</property>"													\
    "<property name=\"opacity\">0.90000000000000002</property>"										\
    "<property name=\"startup_id\">Ardesia</property>"												\
    "<signal name=\"window-state-event\" handler=\"on_bar_window_state_event\" swapped=\"no\"/>"	\
    "<signal name=\"configure-event\" handler=\"on_bar_configure_event\" swapped=\"no\"/>"   	 	\
  "</object>"																						\
"</interface>"



#endif

