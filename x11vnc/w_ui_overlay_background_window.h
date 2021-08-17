#ifndef W_UI_OVERLAY_BACKGROUND_WINDOW_H_
#define W_UI_OVERLAY_BACKGROUND_WINDOW_H_

#include <gtk/gtk.h>

#define BACKGROUND_OPACITY 0.01


/* Structure that contains the info passed to the callbacks. */
typedef struct
{

  /* Gtkbuilder for background window. */
  GtkBuilder *background_window_gtk_builder;

  /* 0 no background, 1 color, 1 image*/
  gint background_type;

  /* Background colour selected. */
  gchar *background_color; 

  /* Background image selected. */
  gchar *background_image;

  /* The background widget that represent the full window. */
  GtkWidget *background_window;

  /* cairo context to draw on the background window. */
  cairo_t *background_cr;

}BackgroundData;


/* Create the background window. */
GtkWidget *
create_background_window     ();

/* Set the background type. */
void
set_background_type     (gint type);


/* Get the background type */
gint
get_background_type     ();


/* Set the background image. */
void
set_background_image    (gchar *background_image);


/* Update the background image. */
void
update_background_image (gchar *name);


/* Get the background image */
gchar *
get_background_image    ();


/* Set the background colour. */
void
set_background_color    (gchar *rgba);


/* Update the background colour. */
void
update_background_color (gchar* rgba);


/* Get the background colour */
gchar *
get_background_color    ();


/* Clear the background. */
void
clear_background_window ();


/* Destroy background window. */
void
destroy_background_window    ();


/* Get the background window. */
GtkWidget *
get_background_window   ();


/* Set the background window. */
void
set_background_window   (GtkWidget *widget);


/* Upgrade the background window */
void
update_background       ();


#define BACKGROUND_UI_GLADE_TEXT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"													\
		"<interface>"																					\
		  "<object class=\"GtkWindow\" id=\"backgroundWindow\">"										\
		    "<property name=\"sensitive\">False</property>"												\
		    "<property name=\"app_paintable\">True</property>"											\
		    "<property name=\"can_focus\">False</property>"												\
		    "<property name=\"events\">GDK_EXPOSURE_MASK</property>"									\
		    "<property name=\"double_buffered\">False</property>"										\
		    "<property name=\"title\">b window</property>"												\
		    "<property name=\"resizable\">False</property>"												\
		    "<property name=\"accept_focus\">False</property>"											\
		    "<property name=\"focus_on_map\">False</property>"											\
		    "<property name=\"decorated\">False</property>"												\
		    "<signal name=\"draw\" handler=\"back_event_expose\" swapped=\"no\"/>"						\
		    "<signal name=\"screen-changed\" handler=\"on_back_screen_changed\" swapped=\"no\"/>"		\
		    "<signal name=\"configure-event\" handler=\"on_back_configure\" swapped=\"no\"/>"			\
		    "<child>"																					\
		      "<placeholder/>"																			\
		    "</child>"																					\
		  "</object>"																					\
		"</interface>"

#endif

