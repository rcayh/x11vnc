
#include "w_ui_overlay_background_window_callbacks.h"
#include "w_ui_overlay_background_window.h"
#include "w_ui_overlay_utils.h"

/* On configure event. */
G_MODULE_EXPORT gboolean
on_back_configure                 (GtkWidget       *widget,
                                   GdkEventExpose  *event,
                                   gpointer        user_data)
{
  BackgroundData *background_data = (BackgroundData *) user_data;

  GdkWindowState state = gdk_window_get_state (gtk_widget_get_window (widget));
  gint is_fullscreen = state & GDK_WINDOW_STATE_FULLSCREEN;
  if (!is_fullscreen)
    {
      return FALSE;
    }

  if (!background_data->background_cr)
    {
      background_data->background_cr = gdk_cairo_create (gtk_widget_get_window (widget) );
    }

  return TRUE;
}


/* On screen changed. */
G_MODULE_EXPORT void
on_back_screen_changed            (GtkWidget  *widget,
                                   GdkScreen  *previous_screen,
                                   gpointer    user_data)
{
  GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET (widget));
  GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
  if (visual == NULL)
    {
      visual = gdk_screen_get_system_visual (screen);
    }
    
  gtk_widget_set_visual (widget, visual);
}


/* Expose event in background window occurs. */
G_MODULE_EXPORT gboolean
back_event_expose                 (GtkWidget  *widget,
                                   cairo_t    *cr,
                                   gpointer user_data)
{
  BackgroundData *background_data = (BackgroundData *) user_data;
  
  if ((background_data->background_image) && (get_background_type () == 2))
    {
      update_background_image (background_data->background_image);
    }
  else if ((background_data->background_color) && (get_background_type () == 1))
    {
      update_background_color (background_data->background_color);
    }
  else
    {
      clear_background_window ();
    }
    /* This allows the mouse event to be passed to the window below. */
#ifndef _WIN32
  cairo_region_t* r = gdk_cairo_region_create_from_surface(cairo_get_target (cr));
  gtk_widget_input_shape_combine_region (widget, r);
  cairo_region_destroy(r);
#endif
  return TRUE;
}


