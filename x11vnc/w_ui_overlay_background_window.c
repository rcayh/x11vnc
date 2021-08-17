

#include "w_ui_overlay_utils.h"
#include "w_ui_overlay_background_window.h"
#include "w_ui_overlay_background_window_callbacks.h"
#include "w_ui_overlay_annotation_window.h"


/* The background data used internally and by the callbacks. */
static BackgroundData *background_data;


/* Load a file image in the window. */
static void
load_file               ()
{
  if (background_data->background_cr)
    {
      cairo_surface_t *surface = cairo_image_surface_create_from_png (background_data->background_image);
      cairo_t *cr = cairo_create (surface);

      gtk_window_set_opacity (GTK_WINDOW (background_data->background_window), 1.0);
	
      gint new_height = 0;
      gint new_width = 0;
      new_height = gdk_window_get_height (gtk_widget_get_window (background_data->background_window));
      new_width = gdk_window_get_width (gtk_widget_get_window (background_data->background_window));

      cairo_surface_t *scaled_surface = scale_surface (surface, new_width, new_height );
	
      cairo_surface_destroy (surface);
	
      cairo_destroy (cr);
	
      cairo_set_source_surface (background_data->background_cr, scaled_surface, 0.0, 0.0);
	
      cairo_paint (background_data->background_cr);
      cairo_stroke (background_data->background_cr);
	
      cairo_surface_destroy (scaled_surface);
	
    }
}


/* The windows has been exposed after the show_all request to change the background color. */
static void
load_color              ()
{
  gint r = 0;
  gint g = 0;
  gint b = 0;
  gint a = 0;

  if (background_data->background_cr)
    {
      sscanf (background_data->background_color, "%02X%02X%02X%02X", &r, &g, &b, &a);

      cairo_set_operator (background_data->background_cr, CAIRO_OPERATOR_SOURCE);


#ifdef WIN32
      gdouble opacity = BACKGROUND_OPACITY;
      cairo_set_source_rgb (background_data->background_cr, (gdouble) r/256, (gdouble) g/256, (gdouble) b/256);

      /*
       * @TODO Implement with a full opaque windows and use cairo_set_source_rgba
       * function to paint.
       * I set the opacity with alpha and I use cairo_set_source_rgb to workaround
       * the problem on windows with rgba. 
       */
       if (((gdouble) a/256) >  BACKGROUND_OPACITY)
         {
           opacity = (gdouble) a/256;
         }
      gtk_window_set_opacity (GTK_WINDOW (background_data->background_window), opacity);
#else
      gtk_window_set_opacity (GTK_WINDOW (background_data->background_window), 1);
      cairo_set_source_rgba (background_data->background_cr,
                             (gdouble) r/256,
                             (gdouble) g/256,
                             (gdouble) b/256,
                             (gdouble) a/256);
#endif

      cairo_paint (background_data->background_cr);
      cairo_stroke (background_data->background_cr);

    }
}


/* Allocate internal structure. */
static BackgroundData *
allocate_background_data          ()
{
  BackgroundData *background_data   = g_malloc ((gsize) sizeof (BackgroundData));
  background_data->background_color = (gchar *) NULL;
  background_data->background_image = (gchar *) NULL;
  background_data->background_cr          = (cairo_t *) NULL;
  background_data->background_window = (GtkWidget *) NULL;
  background_data->background_type = 0;
  return background_data;
}


/* Destroy the background window. */
void
destroy_background_window         ()
{
  if (background_data)
    {

      if (background_data->background_cr)
        {
          cairo_destroy (background_data->background_cr);
          background_data->background_cr = (cairo_t *) NULL;
        }

      if (background_data->background_color)
        {
          g_free (background_data->background_color);
          background_data->background_color = (gchar *) NULL;
        }

      if (background_data->background_window)
        {
          /* Destroy brutally the background window. */
          gtk_widget_destroy (background_data->background_window);
          background_data->background_window = (GtkWidget *) NULL;
        }

      /* Delete reference to the gtk builder object. */
      if (background_data->background_window_gtk_builder)
        {
          g_object_unref (background_data->background_window_gtk_builder);
          background_data->background_window_gtk_builder = (GtkBuilder *) NULL;
        }

      if (background_data)
        {
          g_free (background_data);
          background_data = (BackgroundData *) NULL;
        }

    }
}


/* Clear the background. */
void clear_background_window      ()
{
  /*
   * @HACK Deny the mouse input to go below the window putting the opacity greater than 0
   * I avoid a complete transparent window because in some operating system this would become
   * transparent to the pointer input also.
   *
   */
  gtk_window_set_opacity (GTK_WINDOW (background_data->background_window), BACKGROUND_OPACITY);

  clear_cairo_context (background_data->background_cr);
}


/* Create the background window. */
GtkWidget *
create_background_window     ()
{
  GError *error = (GError *) NULL;
  GObject *background_obj = (GObject *) NULL;

  background_data = allocate_background_data ();

  /* Initialize the background window. */
  background_data->background_window_gtk_builder = gtk_builder_new ();

  /* Load the gtk builder file created with glade. */
  gtk_builder_add_from_string (background_data->background_window_gtk_builder,
		  BACKGROUND_UI_GLADE_TEXT, strlen(BACKGROUND_UI_GLADE_TEXT), &error);

  if (error)
    {
      g_warning ("Failed to load builder file: %s", error->message);
      g_error_free (error);
      return background_data->background_window;
    }

  background_obj = gtk_builder_get_object (background_data->background_window_gtk_builder, "backgroundWindow");
  background_data->background_window = GTK_WIDGET (background_obj);
  gtk_window_set_keep_above (GTK_WINDOW (background_data->background_window), TRUE);

  /* This trys to set an alpha channel. */
  on_back_screen_changed (background_data->background_window, NULL, background_data);
  
  gtk_window_set_opacity (GTK_WINDOW (background_data->background_window), BACKGROUND_OPACITY);
  
  gtk_widget_set_size_request (background_data->background_window, gdk_screen_width (), gdk_screen_height ());

  /* Connect all the callback from gtkbuilder xml file. */
  gtk_builder_connect_signals (background_data->background_window_gtk_builder, (gpointer) background_data);
    
  //gtk_widget_show_all (background_data->background_window);

  /* This put in full screen; this will generate an exposure. */
  gtk_window_fullscreen (GTK_WINDOW (background_data->background_window));
  
  return  background_data->background_window;
}


/* Get the background type */
gint
get_background_type          ()
{
  return background_data->background_type;
}


/* Get the background image */
gchar * 
get_background_image         ()
{
  if (background_data)
    {
      return background_data->background_image;
    }
  return NULL;
}


/* Get the background colour */
gchar * 
get_background_color         ()
{
  return background_data->background_color;
}


/* Set the background type. */
void
set_background_type          (gint type)
{
  background_data->background_type = type;
}


/* Set the background image. */
void
set_background_image         (gchar *name)
{
  background_data->background_image = name;
}


/* Update the background image. */
void
update_background_image      (gchar *name)
{
  set_background_image (name);
  load_file ();
}


/* Set the background colour. */
void
set_background_color         (gchar* rgba)
{
  background_data->background_color = g_strdup_printf ("%s", rgba);
}


/* Update the background colour. */
void
update_background_color      (gchar* rgba)
{
  set_background_color (rgba);
  load_color ();
}


/* Get the background window. */
GtkWidget *
get_background_window        ()
{
  return background_data->background_window;
}


/* Set the background window. */
void
set_background_window        (GtkWidget *widget)
{
  background_data->background_window = widget;
}


