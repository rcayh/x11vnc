
#include "w_ui_overlay_input.h"


/* Add input device. */
static void
add_input_mode_device   (AnnotateData    *data,
                         GdkDevice       *device,
                         GdkInputMode     mode)
{
  if (!data->devdatatable)
    {
      data->devdatatable = g_hash_table_new (NULL, NULL);
    }

  AnnotateDeviceData *devdata = (AnnotateDeviceData *) NULL;
  devdata  = g_malloc ((gsize) sizeof (AnnotateDeviceData));
  devdata->coord_list = (GSList *) NULL;
  g_hash_table_insert (data->devdatatable, device, devdata);
  
  if (!gdk_device_set_mode (device, mode))
    {
      g_warning ("Unable to set the device %s to the %d mode\n",
                  gdk_device_get_name (device),
                  mode);
    }
		
  g_printerr ("Enabled Device in screen %d. %p: \"%s\" (Type: %d)\n",
               mode,
               device,
               gdk_device_get_name (device),
               gdk_device_get_source (device));
}


/* Set-up input device list. */
static void
setup_input_device_list (AnnotateData  *data,
                         GList         *devices)
{
  remove_input_devices (data);
  g_list_foreach (devices, (GFunc) add_input_device, data);
}


/* Select the preferred input mode depending on axis. */
static GdkInputMode
select_input_device_mode     (GdkDevice     *device)
{
  if (gdk_device_get_source (device) != GDK_SOURCE_KEYBOARD && gdk_device_get_n_axes (device) >= 2)
    {
      /* Choose screen mode. */
      return GDK_MODE_SCREEN;
    }
  else
    {
      /* Choose window mode. */
      return GDK_MODE_WINDOW;
    }
}


/* Remove all the devices . */
void
remove_input_devices    (AnnotateData  *data)
{
  if (data->devdatatable)
    {
      GList* list = (GList *) NULL;
      list = g_hash_table_get_keys (data->devdatatable);
      g_list_foreach (list, (GFunc) remove_input_device, data);
      data->devdatatable = (GHashTable *) NULL;
    }
}


/* Set-up input devices. */
void
setup_input_devices     (AnnotateData  *data)
{
  GList *devices;
  GdkDeviceManager *device_manager = gdk_display_get_device_manager (gdk_display_get_default ());
  GList *masters = gdk_device_manager_list_devices (device_manager, GDK_DEVICE_TYPE_MASTER);
  GList *slavers = gdk_device_manager_list_devices (device_manager, GDK_DEVICE_TYPE_SLAVE);
  devices = g_list_concat(masters, slavers);
  setup_input_device_list (data, devices);
}


/* Add input device. */
void
add_input_device        (GdkDevice     *device,
                         AnnotateData  *data)
{
  /* only enable devices with 2 or more axes and exclude keyboards */
  if ((gdk_device_get_source(device) != GDK_SOURCE_KEYBOARD) &&
      ( gdk_device_get_n_axes (device) >= 2))
    {
      add_input_mode_device (data, device, select_input_device_mode (device));
    }
}


/* Remove input device. */
void
remove_input_device     (GdkDevice     *device,
                         AnnotateData  *data)
{
  if (data)
    {
      AnnotateDeviceData *devdata = g_hash_table_lookup (data->devdatatable, device);
      annotate_coord_dev_list_free (devdata);
      g_hash_table_remove (data->devdatatable, device);
    }
}


/* Grab pointer. */
void
grab_pointer       (GtkWidget           *widget,
                    GdkEventMask         eventmask)
{
  GdkGrabStatus result;
  GdkDisplay    *display = (GdkDisplay *) NULL;
  GdkDevice     *pointer = (GdkDevice *) NULL;
  GdkDeviceManager *device_manager = (GdkDeviceManager *) NULL;

  display = gdk_display_get_default ();
  ungrab_pointer     (display);
  device_manager = gdk_display_get_device_manager (display);
  pointer = gdk_device_manager_get_client_pointer (device_manager);

  gdk_error_trap_push ();

  result = gdk_device_grab (pointer,
                            gtk_widget_get_window (widget),
                            GDK_OWNERSHIP_WINDOW,
                            TRUE,
                            eventmask,
                            NULL,
                            GDK_CURRENT_TIME);

  gdk_flush ();
  if (gdk_error_trap_pop ())
    {
      g_printerr ("Grab pointer error\n");
    }

  switch (result)
    {
    case GDK_GRAB_SUCCESS:
      break;
    case GDK_GRAB_ALREADY_GRABBED:
      g_printerr ("Grab Pointer failed: AlreadyGrabbed\n");
      break;
    case GDK_GRAB_INVALID_TIME:
      g_printerr ("Grab Pointer failed: GrabInvalidTime\n");
      break;
    case GDK_GRAB_NOT_VIEWABLE:
      g_printerr ("Grab Pointer failed: GrabNotViewable\n");
      break;
    case GDK_GRAB_FROZEN:
      g_printerr ("Grab Pointer failed: GrabFrozen\n");
      break;
    default:
      g_printerr ("Grab Pointer failed: Unknown error\n");
    }

}


/* Ungrab pointer. */
void
ungrab_pointer     (GdkDisplay        *display)
{
  GdkDevice     *pointer = (GdkDevice *) NULL;
  GdkDeviceManager *device_manager = (GdkDeviceManager *) NULL;

  display = gdk_display_get_default ();
  device_manager = gdk_display_get_device_manager (display);
  pointer = gdk_device_manager_get_client_pointer (device_manager);

  gdk_error_trap_push ();

  gdk_device_ungrab (pointer, GDK_CURRENT_TIME);
  gdk_flush ();
  if (gdk_error_trap_pop ())
    {
      /* this probably means the device table is outdated,
       * e.g. this device doesn't exist anymore.
       */
      g_printerr ("Ungrab pointer device error\n");
    }
}


