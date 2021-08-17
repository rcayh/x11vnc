

#include "w_ui_overlay_utils.h"
#include "w_ui_overlay_bar_callbacks.h"
#include "w_ui_overlay_bar.h"
#include "w_ui_overlay_annotation_window.h"
#include "w_ui_overlay_background_window.h"


/* Timer used to up-rise the window. */
static gint timer = -1;


/* Try to up-rise the window; 
 * this is used for the window manager 
 * that does not support the stay above directive.
 */
static gboolean
bar_to_top         (gpointer data)
{
  if (!gtk_widget_get_visible (GTK_WIDGET (data)))
    {
       gtk_window_present (GTK_WINDOW (data));
       gtk_widget_grab_focus (data);
       gdk_window_lower (gtk_widget_get_window (GTK_WIDGET (data)));
    }
  return TRUE;
}


/* Release to lock the mouse */
static void
release_lock                 (BarData *bar_data)
{
  if (bar_data->grab)
    {
      /* Lock enabled. */
      bar_data->grab = FALSE;
      annotate_release_grab ();

      /* Try to up-rise the window. */
      timer = g_timeout_add (BAR_TO_TOP_TIMEOUT, bar_to_top, get_background_window ());
#ifdef _WIN32 // WIN32
      if (gtk_window_get_opacity (GTK_WINDOW (get_background_window ()))!=0)
        {
          /* 
           * @HACK This allow the mouse input go below the window putting
           * the opacity to 0; when will be found a better way to make
           * the window transparent to the the pointer input we might
           * remove the previous hack.
           * @TODO Transparent window to the pointer input in a better way.
           */
           gtk_window_set_opacity (GTK_WINDOW (get_background_window ()), 0);
        }
#endif

    }
}


/* Lock the mouse. */
static void
lock (BarData *bar_data)
{
  if (! bar_data->grab)
    {
      // Unlock
      bar_data->grab = TRUE;
	
      /* delete the old timer */
      if (timer!=-1)
        {
          g_source_remove (timer);
          timer = -1;
        }

#ifdef _WIN32 // WIN32

      /* 
       * @HACK Deny the mouse input to go below the window putting the opacity greater than 0
       * @TODO remove the opacity hack when will be solved the next todo.
       */
      if (gtk_window_get_opacity (GTK_WINDOW (get_background_window ()))==0)
        {
          gtk_window_set_opacity (GTK_WINDOW (get_background_window ()), BACKGROUND_OPACITY);
        }
#endif
    }
}



/* Windows state event: this occurs when the windows state changes. */
G_MODULE_EXPORT gboolean
on_bar_window_state_event         (GtkWidget            *widget,
                                   GdkEventWindowState  *event,
                                   gpointer              func_data)
{
  BarData *bar_data = (BarData *) func_data;


  g_print("on_bar_window_state_event ==> %d\n", gdk_window_get_state (gtk_widget_get_window (widget)));


  /* Track the minimized signals */
  if(gdk_window_get_state (gtk_widget_get_window (widget)) & GDK_WINDOW_STATE_ICONIFIED)
    {
      release_lock (bar_data);
    }

  return TRUE;
}


/* Configure events occurs. */
G_MODULE_EXPORT gboolean
on_bar_configure_event            (GtkWidget  *widget,
                                   GdkEvent   *event,
                                   gpointer   func_data)
{
  BarData *bar_data = (BarData *) func_data;
  return TRUE;
}


/* Called when push the quit button */
G_MODULE_EXPORT gboolean
on_bar_quit                     (GtkToolButton   *toolbutton,
                                 gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  return FALSE;
}


/* Called when push the info button. */
G_MODULE_EXPORT gboolean
on_bar_info                      (GtkToolButton   *toolbutton,
                                  gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  bar_data->grab = FALSE;
  
  /* Release grab. */
  annotate_release_grab ();

  bar_data->grab = grab_value;
  return TRUE;
}
