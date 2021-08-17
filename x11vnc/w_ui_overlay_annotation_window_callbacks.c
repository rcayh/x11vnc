
#include "w_ui_overlay_annotation_window_callbacks.h"
#include "w_ui_overlay_annotation_window.h"
#include "w_ui_overlay_utils.h"
#include "w_ui_overlay_input.h"


/* Return the pressure passing the event. */
static gdouble
get_pressure       (GdkEvent *ev)
{
  gdouble ret_value = 1.0;
  gdouble pressure = ret_value;

  gboolean ret = gdk_event_get_axis (ev, GDK_AXIS_PRESSURE, &pressure);

  if (ret)
    {
      ret_value = pressure;
    }

  return ret_value;
}


/* On configure event. */
G_MODULE_EXPORT gboolean
on_configure       (GtkWidget      *widget,
                    GdkEventExpose *event,
                    gpointer        user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;

  if (data->debug)
    {
      g_printerr("DEBUG: Annotation window get configure event (%d,%d)\n",
                 gtk_widget_get_allocated_width (widget),
                 gtk_widget_get_allocated_height (widget));
    }
	
  GdkWindowState state = gdk_window_get_state (gtk_widget_get_window (widget));
  gint is_fullscreen = state & GDK_WINDOW_STATE_FULLSCREEN;

  if (!is_fullscreen)
    {
      return FALSE;
    }

  initialize_annotation_cairo_context (data);
  
  if (!data->is_grabbed)
    {
      return FALSE;
    }

  /* Postcondition; data->annotation_cairo_context is not NULL. */
  return TRUE;
}


/* On screen changed. */
G_MODULE_EXPORT void
on_screen_changed       (GtkWidget  *widget,
                         GdkScreen  *previous_screen,
                         gpointer    user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;

  if (data->debug)
    {
      g_printerr ("DEBUG: Annotation window get screen-changed event\n");
    }

  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (widget));
  GdkVisual *visual = gdk_screen_get_rgba_visual (screen);
  
  if (visual == NULL)
    {
      visual = gdk_screen_get_system_visual (screen);
    }

  gtk_widget_set_visual (widget, visual);



}

//yh testcase
void on_overlay_test(GtkWidget *widget, AnnotateData *data){

  GtkWidget *toplevel = gtk_widget_get_toplevel (widget);
  if (gtk_widget_is_toplevel (toplevel))
	{
	  g_print("top level ======================================\n");
	}



  int screen_width, screen_height, window_left, window_top,  window_width, window_height;

  GdkScreen *screen;
  screen = gtk_widget_get_screen(data->annotation_window);
  screen_width = gdk_screen_get_width(screen);
  screen_height = gdk_screen_get_height(screen);

  gtk_window_get_position(data->annotation_window, &window_left, &window_top);
  gtk_window_get_size(data->annotation_window, &window_width, &window_height);


  g_print("annotation window size = screen:(%dx%d) window:(%dx%d) (%dx%d)\n",
		  screen_width, screen_height, window_left, window_top, window_width, window_height);

  gtk_window_move (data->annotation_window, -500, -500);


  gtk_window_get_position(data->annotation_window, &window_left, &window_top);

  g_print("2 annotation window size = screen:(%dx%d) window:(%dx%d) (%dx%d)\n",
			  screen_width, screen_height, window_left, window_top, window_width, window_height);
  /*
  GdkAtom atom;
  GdkAtom cardinal;
  unsigned long strut[12];
  memset(&strut, 0, sizeof(strut));

  strut[0] = -100;



  cardinal = gdk_atom_intern("CARDINAL", FALSE);
  atom = gdk_atom_intern("_NET_WM_STRUT", FALSE);
  gdk_property_change(data->annotation_window, atom, cardinal, 32, GDK_PROP_MODE_REPLACE, (guchar*)strut, 4);
  */
}

/* Expose event: this occurs when the window is shown. */
G_MODULE_EXPORT gboolean
on_expose          (GtkWidget *widget,
                    cairo_t   *cr,
                    gpointer   user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;

  if (data->debug)
    {
      g_printerr ("DEBUG: Annotation window get expose event\n");
    }

  annotate_restore_surface ();

  if(!data->is_onload){
	  annotate_release_input_grab();
	  annotate_redpen_config();
	  data->is_onload = TRUE;
	  //on_overlay_test(widget, data);
  }



  return TRUE;
}


/*
 * Event-Handlers to perform the drawing.
 */


/* This is called when the button is pushed. */
G_MODULE_EXPORT gboolean
on_button_press    (GtkWidget      *win,
                    GdkEventButton *ev,
                    gpointer        user_data)
{

  AnnotateData *data = (AnnotateData *) user_data;
  GdkDevice *master = gdk_event_get_device ( (GdkEvent *) ev);
  
  /* Get the data for this device. */
  AnnotateDeviceData *masterdata = g_hash_table_lookup (data->devdatatable, master);
  
  gdouble pressure = 1.0;
  
  if (data->cur_context == data->default_filler)
    {
      return FALSE;
    }
  
  if (!data->is_grabbed)
    {
      return FALSE;
    }
	
  if (!ev)
    {
      g_printerr ("Device '%s': Invalid event; I ungrab all\n",
                  gdk_device_get_name (master));
      annotate_release_grab ();
      return FALSE;
    }
	
  if (data->debug)
    {
      g_printerr ("Device '%s': Button %i Down at (x,y)= (%f : %f)\n",
                  gdk_device_get_name (master),
                  ev->button,
                  ev->x,
                  ev->y);
    }

#ifdef _WIN32
  if (inside_bar_window (ev->x_root, ev->y_root))
    {
      /* The point is inside the ardesia bar then ungrab. */
      annotate_release_grab ();
      return FALSE;
    }
#endif

  pressure = get_pressure ( (GdkEvent *) ev);

  if (pressure <= 0)
    {
      return FALSE;
    }
	
  annotate_unhide_cursor ();

  initialize_annotation_cairo_context (data);

  annotate_configure_pen_options (data);

  annotate_coord_dev_list_free (masterdata);
  annotate_draw_point (masterdata, ev->x, ev->y, pressure);

  annotate_coord_list_prepend (masterdata,
                               ev->x,
                               ev->y,
                               annotate_get_thickness (),
                               pressure);

  return TRUE;
}


/* This shots when the pointer is moving. */
G_MODULE_EXPORT gboolean
on_motion_notify   (GtkWidget       *win,
                    GdkEventMotion  *ev,
                    gpointer         user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;
  GdkDevice *master = gdk_event_get_device ( (GdkEvent *) ev);
  GdkDevice *slave = gdk_event_get_source_device ( (GdkEvent *) ev);
  
  /* Get the data for this device. */
  AnnotateDeviceData *masterdata= g_hash_table_lookup (data->devdatatable, master);
  AnnotateDeviceData *slavedata = g_hash_table_lookup (data->devdatatable, slave);

   if (data->cur_context == data->default_filler)
    {
      return FALSE;
    }
    
  if (ev->state != masterdata->state ||
      ev->state != slavedata->state  ||
      masterdata->lastslave != slave)
    {
       annotate_select_tool (data, master, slave, ev->state);
    }

  gdouble selected_width = 0.0;
  gdouble pressure = 1.0; 

  if (!data->is_grabbed)
    {
      return FALSE;
    }

  if (!ev)
    {
      g_printerr ("Device '%s': Invalid event; I ungrab all\n",
                  gdk_device_get_name (master));
      annotate_release_grab ();
      return FALSE;
    }

  if (data->debug)
    {
      g_printerr ("Device '%s': Move at (x,y)= (%f : %f)\n",
                  gdk_device_get_name (master),
                  ev->x,
                  ev->y);
    }
  
#ifdef _WIN32
  if (inside_bar_window (ev->x_root, ev->y_root))
    {

      if (data->debug)
        {
          g_printerr ("Device '%s': Move on the bar then ungrab\n",
                       gdk_device_get_name (master));
        }

      /* The point is inside the ardesia bar then ungrab. */
      annotate_release_grab ();
      return FALSE;
    }
#endif

  annotate_unhide_cursor ();
  
  /* Only the first 5 buttons allowed. */
  if(!(ev->state & (GDK_BUTTON1_MASK|
                    GDK_BUTTON2_MASK|
                    GDK_BUTTON3_MASK|
                    GDK_BUTTON4_MASK|
                    GDK_BUTTON5_MASK)))
    {
      return TRUE;
    }

  initialize_annotation_cairo_context (data);

  annotate_configure_pen_options (data);
  
  if (data->cur_context->type != ANNOTATE_ERASER)
    {
      pressure = get_pressure ( (GdkEvent *) ev);

      if (pressure <= 0)
        {
          return FALSE;
        }

      /* If the point is already selected and higher pressure then print else jump it. */
      if (masterdata->coord_list)
        {
          AnnotatePoint *last_point = (AnnotatePoint *) g_slist_nth_data (masterdata->coord_list, 0);
          gdouble tollerance = annotate_get_thickness ();

          if (get_distance (last_point->x, last_point->y, ev->x, ev->y)<tollerance)
            {
              /* Seems that you are uprising the pen. */
              if (pressure <= last_point->pressure)
                {
                  /* Jump the point you are uprising the hand. */
                  return FALSE;
                }
              else // pressure >= last_point->pressure
                {
                  /* Seems that you are pressing the pen more. */
                  annotate_modify_color (masterdata, data, pressure);
                  annotate_draw_line (masterdata, ev->x, ev->y, TRUE);
                  /* Store the new pressure without allocate a new coordinate. */
                  last_point->pressure = pressure;
                  return TRUE;
                }
            }
          annotate_modify_color (masterdata, data, pressure);
        }
    }
    
  annotate_draw_line (masterdata, ev->x, ev->y, TRUE);
  annotate_coord_list_prepend (masterdata, ev->x, ev->y, selected_width, pressure);

  return TRUE;
}


/* This shots when the button is released. */
G_MODULE_EXPORT gboolean
on_button_release  (GtkWidget       *win,
                    GdkEventButton  *ev,
                    gpointer         user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;
  
  GdkDevice *master = gdk_event_get_device ( (GdkEvent *) ev);
  
  /* Get the data for this device. */
  AnnotateDeviceData *masterdata= g_hash_table_lookup (data->devdatatable, master);
  
  guint lenght = g_slist_length (masterdata->coord_list);

  if (!data->is_grabbed)
    {
      return FALSE;
    }
	
  if (!ev)
    {
      g_printerr ("Device '%s': Invalid event; I ungrab all\n",
                  gdk_device_get_name (master));
      annotate_release_grab ();
      return FALSE;
    }

  if (data->debug)
    {
      g_printerr ("Device '%s': Button %i Up at (x,y)= (%.2f : %.2f)\n",
                  gdk_device_get_name (master),
                   ev->button, ev->x, ev->y);
    }

#ifdef _WIN32
  if (inside_bar_window (ev->x_root, ev->y_root))
    /* Point is in the ardesia bar. */
    {
      /* The last point was outside the bar then ungrab. */
      annotate_release_grab ();
      return FALSE;
    }
  if (data->old_paint_type == ANNOTATE_PEN)
    {
       annotate_select_pen ();
    }
#endif

  if (data->cur_context == data->default_filler)
    {
      annotate_fill (masterdata, data, ev->x, ev->y);
      return TRUE;
    }
    
  initialize_annotation_cairo_context (data);

  if (lenght > 2)
    {
      AnnotatePoint *first_point = (AnnotatePoint *) g_slist_nth_data (masterdata->coord_list, lenght-1);
      AnnotatePoint *last_point = (AnnotatePoint *) g_slist_nth_data (masterdata->coord_list, 0);

      gdouble distance = get_distance (ev->x, ev->y, first_point->x, first_point->y);

      /* This is the tolerance to force to close the path in a magnetic way. */
      gint score = 3;
      
      /* If is applied some handled drawing mode then the tool is more tollerant. */
      if ((data->rectify || data->roundify))
        {
          score = 6;
        }
        
      gdouble tollerance = annotate_get_thickness () * score;

      gdouble pressure = last_point->pressure;
      annotate_modify_color (masterdata, data, pressure);

      gboolean closed_path = FALSE;

      /* If the distance between two point lesser than tolerance they are the same point for me. */
      if (distance > tollerance)
        {
          /* Different point. */
          annotate_draw_line (masterdata, ev->x, ev->y, TRUE);
          annotate_coord_list_prepend (masterdata, ev->x, ev->y, annotate_get_thickness (), pressure);
        }
      else
        {
          /* Rounded to be the same point. */
          closed_path = TRUE; // this seems to be a closed path
          annotate_draw_line (masterdata, first_point->x, first_point->y, TRUE);
          annotate_coord_list_prepend (masterdata, first_point->x, first_point->y, annotate_get_thickness (), pressure);
        }

      if (data->cur_context->type != ANNOTATE_ERASER)
        {
          annotate_shape_recognize (masterdata, closed_path);

          /* If is selected an arrow type then I draw the arrow. */
          if (data->arrow)
            {
              /* Print arrow at the end of the path. */
              annotate_draw_arrow (masterdata, distance);
            }
        }
    }

  cairo_stroke (data->annotation_cairo_context);

  annotate_add_savepoint ();

  annotate_hide_cursor ();

  return TRUE;
}


/* On device added. */
void on_device_removed  (GdkDeviceManager  *device_manager,
                         GdkDevice         *device,
                         gpointer           user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;

  if(data->debug)
    {
      g_printerr ("DEBUG: device '%s' removed\n", gdk_device_get_name(device));
    }
    
  remove_input_device (device, data);
}


/* On device removed. */
void on_device_added    (GdkDeviceManager  *device_manager,
                         GdkDevice         *device,
                         gpointer           user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;

  if(data->debug)
    {
      g_printerr ("DEBUG: device '%s' added\n", gdk_device_get_name (device));
    }

  add_input_device (device, data);
}

