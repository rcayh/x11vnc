#include "w_ui_overlay_annotation_window.h"
#include "w_ui_overlay_annotation_window_callbacks.h"
#include "w_ui_overlay_utils.h"
#include "w_ui_overlay_input.h"
#include "w_ui_overlay_background_window.h"

#ifdef _WIN32
#  include <windows_utils.h>
#endif

/* Internal data for the annotation window. */
static AnnotateData *data;

/* Create a new paint context. */
static AnnotatePaintContext *
annotate_paint_context_new(AnnotatePaintType type) {
	AnnotatePaintContext *context = (AnnotatePaintContext *) NULL;
	context = g_malloc((gsize) sizeof(AnnotatePaintContext));
	context->type = type;

	return context;
}

/* Calculate the direction in radiant. */
static gdouble annotate_get_arrow_direction(AnnotateDeviceData *devdata) {

	return 0;
}

/* Colour selector; if eraser than select the transparent colour else allocate the right colour. */
static void select_color() {
	if (!data->annotation_cairo_context) {
		return;
	}

	if (data->cur_context) {
		if (data->cur_context->type != ANNOTATE_ERASER) //pen or arrow tool
				{
			/* Select the colour. */
			if (data->color) {
				if (data->debug) {
					g_printerr("Select colour %s\n", data->color);
				}

				cairo_set_source_color_from_string(
						data->annotation_cairo_context, data->color);
			}

			cairo_set_operator(data->annotation_cairo_context,
					CAIRO_OPERATOR_SOURCE);
		} else {

			/* It is the eraser tool. */
			if (data->debug) {
				g_printerr("Select transparent colour to erase\n");
			}

			cairo_set_operator(data->annotation_cairo_context,
					CAIRO_OPERATOR_CLEAR);
		}
	}
}

#ifdef _WIN32

/* Acquire the grab pointer. */
static void
annotate_acquire_pointer_grab ()
{
	grab_pointer (data->annotation_window, GDK_ALL_EVENTS_MASK);
}

/* Release the grab pointer. */
static void
annotate_release_pointer_grab ()
{
	ungrab_pointer (gdk_display_get_default ());
}

#endif

/* Update the cursor icon. */
static void update_cursor() {
	if (!data->annotation_window) {
		return;
	}

#ifdef _WIN32
	annotate_release_pointer_grab ();
#endif

	gdk_window_set_cursor(gtk_widget_get_window(data->annotation_window),
			data->cursor);

#ifdef _WIN32
	annotate_acquire_pointer_grab ();
#endif
}

/* Dis-allocate cursor. */
static void disallocate_cursor() {
	if (data->cursor) {
		g_object_unref(data->cursor);
		data->cursor = (GdkCursor *) NULL;
	}
}

/* Take the input mouse focus. */
static void annotate_acquire_input_grab() {
#ifdef _WIN32
	grab_pointer (data->annotation_window, GDK_ALL_EVENTS_MASK);
#endif

#ifndef _WIN32
	/*
	 * MACOSX; will do nothing.
	 */
	gtk_widget_input_shape_combine_region(data->annotation_window, NULL);
	drill_window_in_bar_area(data->annotation_window);
#endif

}

/* Destroy cairo context. */
static void destroy_cairo() {
	guint refcount = (guint) cairo_get_reference_count(
			data->annotation_cairo_context);

	guint i = 0;

	for (i = 0; i < refcount; i++) {
		cairo_destroy(data->annotation_cairo_context);
	}

	data->annotation_cairo_context = (cairo_t *) NULL;
}

/* This an ellipse taking the top left edge coordinates
 * and the width and the height of the bounded rectangle.
 */
static void annotate_draw_ellipse(AnnotateDeviceData *devdata, gdouble x,
		gdouble y, gdouble width, gdouble height, gdouble pressure) {
	if (data->debug) {
		g_printerr("Draw ellipse: 2a=%f 2b=%f\n", width, height);
	}

	annotate_modify_color(devdata, data, pressure);

	cairo_save(data->annotation_cairo_context);

	/* The ellipse is done as a 360 degree arc translated. */
	cairo_translate(data->annotation_cairo_context, x + width / 2.,
			y + height / 2.);
	cairo_scale(data->annotation_cairo_context, width / 2., height / 2.);
	cairo_arc(data->annotation_cairo_context, 0., 0., 1., 0., 2 * M_PI);
	cairo_restore(data->annotation_cairo_context);

}

/* Draw a curve using a cubic bezier splines passing to the list's coordinate. */
static void annotate_draw_curve(AnnotateDeviceData *devdata, GSList *list) {
	guint lenght = g_slist_length(list);

	if (list) {
		guint i = 0;
		for (i = 0; i < lenght; i = i + 3) {
			AnnotatePoint *first_point = (AnnotatePoint *) g_slist_nth_data(
					list, i);
			if (!first_point) {
				return;
			}
			if (lenght == 1) {
				/* It is a point. */
				annotate_draw_point(devdata, first_point->x, first_point->y,
						first_point->pressure);
			} else {
				AnnotatePoint *second_point =
						(AnnotatePoint *) g_slist_nth_data(list, i + 1);
				if (!second_point) {
					return;
				} else {
					AnnotatePoint *third_point =
							(AnnotatePoint *) g_slist_nth_data(list, i + 2);
					if (!third_point) {
						/* draw line from first to second point */
						annotate_draw_line(devdata, second_point->x,
								second_point->y, FALSE);
						return;
					}
					annotate_modify_color(devdata, data,
							second_point->pressure);
					cairo_curve_to(data->annotation_cairo_context,
							first_point->x, first_point->y, second_point->x,
							second_point->y, third_point->x, third_point->y);
				}
			}
		}
	}
}

/* Rectify the line. */
static void rectify(AnnotateDeviceData *devdata, gboolean closed_path) {

}

/* Roundify the line. */
static void roundify(AnnotateDeviceData *devdata, gboolean closed_path) {

}

/* Create the annotation window. */
static GtkWidget *
create_annotation_window() {
	GtkWidget* widget = (GtkWidget *) NULL;
	GError* error = (GError *) NULL;

	/* Initialize the main window. */
	data->annotation_window_gtk_builder = gtk_builder_new();

	/* Load the gtk builder file created with glade. */
	gtk_builder_add_from_string(data->annotation_window_gtk_builder,
			ANNOTATION_UI_GLADE_TEXT, strlen(ANNOTATION_UI_GLADE_TEXT), &error);

	if (error) {
		g_warning("Failed to load builder file: %s", error->message);
		g_error_free(error);
		return widget;
	}

	widget = GTK_WIDGET(
			gtk_builder_get_object(data->annotation_window_gtk_builder,
					"annotationWindow"));

	return widget;
}

/* Set-up the application. */
static void setup_app(GtkWidget* parent) {
	gint width = gdk_screen_width();
	gint height = gdk_screen_height();

	/* Create the annotation window. */
	data->annotation_window = create_annotation_window();

	/* This trys to set an alpha channel. */
	on_screen_changed(data->annotation_window, NULL, data);

	/* Put the opacity to 0 to avoid the initial flickering. */
	gtk_window_set_opacity(GTK_WINDOW(data->annotation_window), 0.0);

	if (data->annotation_window == NULL) {
		g_warning("Failed to create the annotation window");
		return;
	}

	gtk_widget_set_size_request(data->annotation_window, width, height);

	/* Connect all the callback from gtkbuilder xml file. */
	gtk_builder_connect_signals(data->annotation_window_gtk_builder,
			(gpointer) data);

	/* Connet some extra callbacks in order to handle the hotplugged input devices. */
	g_signal_connect(gdk_display_get_device_manager(gdk_display_get_default()),
			"device-added", G_CALLBACK(on_device_added), data);
	g_signal_connect(gdk_display_get_device_manager(gdk_display_get_default()),
			"device-removed", G_CALLBACK(on_device_removed), data);

	/* This show the window in fullscreen generating an exposure. */
	gtk_window_fullscreen(GTK_WINDOW(data->annotation_window));

#ifdef _WIN32
	/* @TODO Use RGBA colormap and avoid to use the layered window. */
	/* I use a layered window that use the black as transparent colour. */
	setLayeredGdkWindowAttributes (gtk_widget_get_window (data->annotation_window),
			RGB (0,0,0),
			0,
			LWA_COLORKEY);
#endif
}

/* Create the directory where put the save-point files. */
static void create_savepoint_dir() {

}

/* Delete the save-point. */
static void delete_savepoint(AnnotateSavepoint *savepoint) {

}

/* Free the list of the  save-point for the redo. */
static void annotate_redolist_free() {
	guint i = data->current_save_index;
	GSList *stop_list = g_slist_nth(data->savepoint_list, i);

	while (data->savepoint_list != stop_list) {
		AnnotateSavepoint *savepoint = (AnnotateSavepoint *) g_slist_nth_data(
				data->savepoint_list, 0);
		delete_savepoint(savepoint);
	}
}

/* Free the list of all the save-point. */
static void annotate_savepoint_list_free() {
	g_slist_foreach(data->savepoint_list, (GFunc) delete_savepoint,
			(gpointer) NULL);

	data->savepoint_list = (GSList *) NULL;
}

/* Delete the ardesia temporary directory */
static void delete_ardesia_tmp_dir() {

}

/* Draw an arrow starting from the point 
 * whith the width and the direction in radiant
 */
static void draw_arrow_in_point(AnnotatePoint *point, gdouble width,
		gdouble direction) {

	gdouble width_cos = width * cos(direction);
	gdouble width_sin = width * sin(direction);

	/* Vertex of the arrow. */
	gdouble arrow_head_0_x = point->x + width_cos;
	gdouble arrow_head_0_y = point->y + width_sin;

	/* Left point. */
	gdouble arrow_head_1_x = point->x - width_cos + width_sin;
	gdouble arrow_head_1_y = point->y - width_cos - width_sin;

	/* Origin. */
	gdouble arrow_head_2_x = point->x - 0.8 * width_cos;
	gdouble arrow_head_2_y = point->y - 0.8 * width_sin;

	/* Right point. */
	gdouble arrow_head_3_x = point->x - width_cos - width_sin;
	gdouble arrow_head_3_y = point->y + width_cos - width_sin;

	cairo_stroke(data->annotation_cairo_context);
	cairo_save(data->annotation_cairo_context);

	/* Initialize cairo properties. */
	cairo_set_line_join(data->annotation_cairo_context, CAIRO_LINE_JOIN_MITER);
	cairo_set_operator(data->annotation_cairo_context, CAIRO_OPERATOR_SOURCE);
	cairo_set_line_width(data->annotation_cairo_context, width);

	/* Draw the arrow. */
	cairo_move_to(data->annotation_cairo_context, arrow_head_2_x,
			arrow_head_2_y);
	cairo_line_to(data->annotation_cairo_context, arrow_head_1_x,
			arrow_head_1_y);
	cairo_line_to(data->annotation_cairo_context, arrow_head_0_x,
			arrow_head_0_y);
	cairo_line_to(data->annotation_cairo_context, arrow_head_3_x,
			arrow_head_3_y);

	cairo_close_path(data->annotation_cairo_context);
	cairo_fill_preserve(data->annotation_cairo_context);
	cairo_stroke(data->annotation_cairo_context);
	cairo_restore(data->annotation_cairo_context);

	if (data->debug) {
		g_printerr("with vertex at (x,y)= (%f : %f)\n", arrow_head_0_x,
				arrow_head_0_y);
	}
}

void destroy_annotation_window() {

	if (data) {

		if (data->annotation_cairo_context) {
			cairo_destroy(data->annotation_cairo_context);
			data->annotation_cairo_context = (cairo_t *) NULL;
		}

		if (data->color) {
			g_free(data->color);
			data->color = (gchar *) NULL;
		}

		if (data->annotation_window) {
			/* Destroy brutally the background window. */
			gtk_widget_destroy(data->annotation_window);
			data->annotation_window = (GtkWidget *) NULL;
		}

		/* Delete reference to the gtk builder object. */
		if (data->annotation_window_gtk_builder) {
			g_object_unref(data->annotation_window_gtk_builder);
			data->annotation_window_gtk_builder = (GtkBuilder *) NULL;
		}

		if (data->penInfo) {
			free(data->penInfo);
			data->penInfo = NULL;
		}

		if (data) {
			g_free(data);
			data = (BackgroundData *) NULL;
		}

	}

}

/* Configure pen option for cairo context. */
void annotate_configure_pen_options(AnnotateData *data) {

	if (data->annotation_cairo_context) {
		cairo_new_path(data->annotation_cairo_context);
		cairo_set_line_cap(data->annotation_cairo_context,
				CAIRO_LINE_CAP_ROUND);
		cairo_set_line_join(data->annotation_cairo_context,
				CAIRO_LINE_JOIN_ROUND);

		if (data->cur_context->type == ANNOTATE_ERASER) {
			data->cur_context = data->default_eraser;
			cairo_set_operator(data->annotation_cairo_context,
					CAIRO_OPERATOR_CLEAR);
			cairo_set_line_width(data->annotation_cairo_context,
					annotate_get_thickness());
		} else {
			cairo_set_operator(data->annotation_cairo_context,
					CAIRO_OPERATOR_SOURCE);
			cairo_set_line_width(data->annotation_cairo_context,
					annotate_get_thickness());
		}
	}
	select_color();
}

/*
 * Add a save point for the undo/redo;
 * this code must be called at the end of each painting action.
 */
void annotate_add_savepoint() {

}

/* Initialize the annotation cairo context */
void initialize_annotation_cairo_context(AnnotateData *data) {
	if (data->annotation_cairo_context == NULL) {
		/* Initialize a transparent window. */
#ifdef _WIN32
		/* The hdc has depth 32 and the technology is DT_RASDISPLAY. */
		HDC hdc = GetDC (GDK_WINDOW_HWND (gtk_widget_get_window (data->annotation_window)));
		/*
		 * @TODO Use an HDC that support the ARGB32 format to support the alpha channel;
		 * this might fix the highlighter bug.
		 * In the documentation is written that the now the resulting surface is in RGB24 format.
		 *
		 */
		cairo_surface_t *surface = cairo_win32_surface_create (hdc);

		data->annotation_cairo_context = cairo_create (surface);
#else
		data->annotation_cairo_context = gdk_cairo_create(
				gtk_widget_get_window(data->annotation_window));
#endif

		if (cairo_status(data->annotation_cairo_context)
				!= CAIRO_STATUS_SUCCESS) {
			g_printerr("Failed to allocate the annotation cairo context");
			annotate_quit();
			exit(EXIT_FAILURE);
		}

		if (data->savepoint_list == NULL) {
			/* Clear the screen and create the first empty savepoint. */
			annotate_clear_screen();
		}
#ifndef _WIN32
		gtk_window_set_opacity(GTK_WINDOW(data->annotation_window), 1.0);
#endif	
		annotate_acquire_grab();

	}
}

/* Draw the last save point on the window restoring the surface. */
void annotate_restore_surface() {

	if (data->debug) {
		g_printerr("Restore surface\n");
	}

	if (data->annotation_cairo_context) {
		guint i = data->current_save_index;

		if (g_slist_length(data->savepoint_list) == i) {
			cairo_new_path(data->annotation_cairo_context);
			clear_cairo_context(data->annotation_cairo_context);
			return;
		}

		AnnotateSavepoint *savepoint = (AnnotateSavepoint *) g_slist_nth_data(
				data->savepoint_list, i);

		if (!savepoint) {
			return;
		}

		cairo_new_path(data->annotation_cairo_context);
		cairo_set_operator(data->annotation_cairo_context,
				CAIRO_OPERATOR_SOURCE);

		if (savepoint->filename) {
			/* Load the file in the annotation surface. */
			cairo_surface_t *image_surface =
					cairo_image_surface_create_from_png(savepoint->filename);
			if (data->debug) {
				g_printerr("The save-point %s has been loaded from file\n",
						savepoint->filename);
			}

			if (image_surface) {
				cairo_set_source_surface(data->annotation_cairo_context,
						image_surface, 0, 0);
				cairo_paint(data->annotation_cairo_context);
				cairo_stroke(data->annotation_cairo_context);
				cairo_surface_destroy(image_surface);
			}

		}
	}
}

/* Get the annotation window. */
GtkWidget *
get_annotation_window() {
	return data->annotation_window;
}

/* Set colour. */
void annotate_set_color(gchar *color) {
	data->color = color;
}

/* Set rectifier. */
void annotate_set_rectifier(gboolean rectify) {
	data->rectify = rectify;
}

/* Set rounder. */
void annotate_set_rounder(gboolean roundify) {
	data->roundify = roundify;
}

/* Set arrow. */
void annotate_set_arrow(gboolean arrow) {
	data->arrow = arrow;
}

/* Set the line thickness. */
void annotate_set_thickness(gdouble thickness) {
	data->thickness = thickness;
}

/* Get the line thickness. */
gdouble annotate_get_thickness() {
	if (data->cur_context->type == ANNOTATE_ERASER) {
		/* the eraser is bigger than pen */
		gdouble corrective_factor = 2.5;
		return data->thickness * corrective_factor;
	}

	return data->thickness;
}

/* Add to the list of the painted point the point (x,y). */
void annotate_coord_list_prepend(AnnotateDeviceData *devdata, gdouble x,
		gdouble y, gdouble width, gdouble pressure) {
	AnnotatePoint *point = g_malloc((gsize) sizeof(AnnotatePoint));
	point->x = x;
	point->y = y;
	point->width = width;
	point->pressure = pressure;
	devdata->coord_list = g_slist_prepend(devdata->coord_list, point);
}

/* Free the coord list belonging to the the owner devdata device. */
void annotate_coord_dev_list_free(AnnotateDeviceData *devdata) {
	if (devdata->coord_list) {
		g_slist_foreach(devdata->coord_list, (GFunc) g_free, (gpointer) NULL);
		g_slist_free(devdata->coord_list);
		devdata->coord_list = (GSList *) NULL;
	}
}

/* Modify colour according to the pressure. */
void annotate_modify_color(AnnotateDeviceData *devdata, AnnotateData *data,
		gdouble pressure) {
	/* Pressure value is from 0 to 1;this value modify the RGBA gradient. */
	guint r, g, b, a;
	gdouble old_pressure = pressure;

	/* If you put an higher value you will have more contrast
	 * between the lighter and darker colour depending on pressure.
	 */
	gdouble contrast = 96;
	gdouble corrective = 0;

	/* The pressure is greater than 0. */
	if ((!data->annotation_cairo_context) || (!data->color)) {
		return;
	}

	if (pressure >= 1) {
		cairo_set_source_color_from_string(data->annotation_cairo_context,
				data->color);
	} else if (pressure <= 0.1) {
		pressure = 0.1;
	}

	sscanf(data->color, "%02X%02X%02X%02X", &r, &g, &b, &a);

	if (devdata->coord_list != NULL) {
		AnnotatePoint *last_point = (AnnotatePoint *) g_slist_nth_data(
				devdata->coord_list, 0);
		old_pressure = last_point->pressure;
	}

	corrective = (1 - (3 * pressure + old_pressure) / 4) * contrast;
	cairo_set_source_rgba(data->annotation_cairo_context,
			(r + corrective) / 255, (g + corrective) / 255,
			(b + corrective) / 255, (gdouble) a / 255);
}

/* Paint the context over the annotation window. */
void annotate_push_context(cairo_t * cr) {
	cairo_surface_t* source_surface = (cairo_surface_t *) NULL;

	if (data->debug) {
		g_printerr(
				"The text window content has been painted over the annotation window\n");
	}

	cairo_new_path(data->annotation_cairo_context);
	source_surface = cairo_get_target(cr);

#ifndef _WIN32
	/*
	 * The over operator might put the new layer on the top of the old one
	 * overriding the intersections
	 * Seems that this operator does not work on windows
	 * in this operating system only the new layer remain after the merge.
	 *
	 */
	cairo_set_operator(data->annotation_cairo_context, CAIRO_OPERATOR_OVER);
#else
	/*
	 * @WORKAROUND using CAIRO_OPERATOR_ADD instead of CAIRO_OPERATOR_OVER
	 * I do this to use the text widget in windows
	 * I use the CAIRO_OPERATOR_ADD that put the new layer
	 * on the top of the old;this function does not preserve the colour of
	 * the second layer but modify it respecting the first layer.
	 *
	 * Seems that the CAIRO_OPERATOR_OVER does not work because in the
	 * gtk cairo implementation the ARGB32 format is not supported.
	 *
	 */
	cairo_set_operator (data->annotation_cairo_context, CAIRO_OPERATOR_ADD);
#endif

	cairo_set_source_surface(data->annotation_cairo_context, source_surface, 0,
			0);
	cairo_paint(data->annotation_cairo_context);
	cairo_stroke(data->annotation_cairo_context);
	annotate_add_savepoint();
}

/* Select the default pen tool. */
void annotate_select_pen() {

}

/* Select the default filler tool. */
void annotate_select_filler() {

}

/* Select the default eraser tool. */
void annotate_select_eraser() {

}

/* Unhide the cursor. */
void annotate_unhide_cursor() {
	if (data->is_cursor_hidden) {
		update_cursor();
		data->is_cursor_hidden = FALSE;
	}
}

/* Hide the cursor icon. */
void annotate_hide_cursor() {
	gdk_window_set_cursor(gtk_widget_get_window(data->annotation_window),
			data->invisible_cursor);

	data->is_cursor_hidden = TRUE;
}

/* acquire the grab. */
void annotate_acquire_grab() {
	ungrab_pointer(gdk_display_get_default());
	if (!data->is_grabbed) {

		if (data->debug) {
			g_printerr("Acquire grab\n");
		}

		annotate_acquire_input_grab();
		data->is_grabbed = TRUE;
	}
}

/* Draw line from the last point drawn to (x2,y2);
 * if stroke is false the cairo path is not forgotten
 */
void annotate_draw_line(AnnotateDeviceData *devdata, gdouble x2, gdouble y2,
		gboolean stroke) {
	if (!stroke) {
		cairo_line_to(data->annotation_cairo_context, x2, y2);
	} else {
		AnnotatePoint *last_point = (AnnotatePoint *) g_slist_nth_data(
				devdata->coord_list, 0);
		if (last_point) {
			cairo_move_to(data->annotation_cairo_context, last_point->x,
					last_point->y);
		} else {
			cairo_move_to(data->annotation_cairo_context, x2, y2);
		}
		cairo_line_to(data->annotation_cairo_context, x2, y2);
		cairo_stroke(data->annotation_cairo_context);
	}
}

/* Draw the point list. */
void annotate_draw_point_list(AnnotateDeviceData *devdata, GSList *list) {
	if (list) {
		guint i = 0;
		guint lenght = g_slist_length(list);
		for (i = 0; i < lenght; i = i + 1) {
			AnnotatePoint *point = (AnnotatePoint *) g_slist_nth_data(list, i);
			if (!point) {
				return;
			}

			if (lenght == 1) {
				/* It is a point. */
				annotate_draw_point(devdata, point->x, point->y,
						point->pressure);
				break;
			}
			annotate_modify_color(devdata, data, point->pressure);
			/* Draw line between the two points. */
			annotate_draw_line(devdata, point->x, point->y, FALSE);
		}
	}
}

/* Draw an arrow using some polygons. */
void annotate_draw_arrow(AnnotateDeviceData *devdata, gdouble distance) {
	gdouble direction = 0;
	gdouble pen_width = annotate_get_thickness();
	gdouble arrow_minimum_size = pen_width * 2;

	AnnotatePoint *point = (AnnotatePoint *) g_slist_nth_data(
			devdata->coord_list, 0);

	if (distance < arrow_minimum_size) {
		return;
	}

	if (data->debug) {
		g_printerr("Draw arrow: ");
	}

	if (g_slist_length(devdata->coord_list) < 2) {
		/* If it has length lesser then two then is a point and it has no sense draw the arrow. */
		return;
	}

	/* Postcondition length >= 2 */
	direction = annotate_get_arrow_direction(devdata);

	if (data->debug) {
		g_printerr("Arrow direction %f\n", direction / M_PI * 180);
	}

	draw_arrow_in_point(point, pen_width, direction);
}

/* Fill the contiguos area around point with coordinates (x,y). */
void annotate_fill(AnnotateDeviceData *devdata, AnnotateData *data, gdouble x,
		gdouble y) {

}

/* Draw a point in x,y respecting the context. */
void annotate_draw_point(AnnotateDeviceData *devdata, gdouble x, gdouble y,
		gdouble pressure) {
	/* Modify a little bit the colour depending on pressure. */
	annotate_modify_color(devdata, data, pressure);
	cairo_move_to(data->annotation_cairo_context, x, y);
	cairo_line_to(data->annotation_cairo_context, x, y);
}

/* Call the geometric shape recognizer. */
void annotate_shape_recognize(AnnotateDeviceData *devdata, gboolean closed_path) {
	if (data->rectify) {
		rectify(devdata, closed_path);
	} else if (data->roundify) {
		roundify(devdata, closed_path);
	}
}

/* Select eraser, pen or other tool for tablet. */
void annotate_select_tool(AnnotateData *data, GdkDevice *masterdevice,
		GdkDevice *slavedevice, guint state) {
	AnnotateDeviceData *masterdata = g_hash_table_lookup(data->devdatatable,
			masterdevice);
	AnnotateDeviceData *slavedata = g_hash_table_lookup(data->devdatatable,
			slavedevice);

	if (slavedevice) {
		if (gdk_device_get_source(slavedevice) == GDK_SOURCE_ERASER) {
			annotate_select_eraser();
			data->old_paint_type = ANNOTATE_PEN;
		} else {
			if (data->old_paint_type == ANNOTATE_ERASER) {
				annotate_select_eraser();
			} else {
				annotate_select_pen();
			}
		}

	} else {
		g_printerr("Attempt to select non existent device!\n");
		data->cur_context = data->default_pen;
	}

	masterdata->lastslave = slavedevice;
	masterdata->state = state;
	slavedata->state = state;
}

/* Free the memory allocated by paint context */
void annotate_paint_context_free(AnnotatePaintContext *context) {
	if (context) {
		g_free(context);
		context = (AnnotatePaintContext *) NULL;
	}
}

/* Quit the annotation. */
void annotate_quit() {

}

void annotate_redpen_draw_test() {

	int quarter_w = gdk_screen_width() / 4;
	int quarter_h = gdk_screen_height() / 4;

	cairo_t * cr = data->annotation_cairo_context;
	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	cairo_set_line_width(cr, 5);
	cairo_move_to(cr, 0, 0);
	cairo_line_to(cr, 300, 300);
	cairo_stroke(cr);
}

void annotate_set_redpen_info(RedpenInfo penInfo) {

	LOCK(drawMutex);

	if (data->penInfo) {
		free(data->penInfo);
		data->penInfo = NULL;
	}

	data->penInfo = (RedpenInfoPtr*) malloc(sizeof(RedpenInfo));
	data->penInfo->PenType = penInfo.PenType;
	data->penInfo->width = penInfo.width;
	data->penInfo->downX = 0;
	data->penInfo->downY = 0;
	data->penInfo->erase = penInfo.erase;
	data->penInfo->windowPosX = 0;
	data->penInfo->windowPosY = 0;
	data->penInfo->color = penInfo.color;

	UNLOCK(drawMutex);

}
void annotate_redpen_config() {

	LOCK(drawMutex);

	cairo_t * cr = data->annotation_cairo_context;
	if(cr){



		int r = (data->penInfo->color)&0xFF;
		int g = (data->penInfo->color>>8)&0xFF;
		int b = (data->penInfo->color>>16)&0xFF;
		//not set
		int a = (data->penInfo->color>>24)&0xFF;

		double c_r = r/255.;
		double c_g = g/255.;
		double c_b = b/255.;
		double c_a = a/255.;

		cairo_set_source_rgb(cr, c_r, c_g, c_b);
		cairo_set_line_width(cr, data->penInfo->width);

		if (data->penInfo->erase == 1) {
			annotate_clear_screen();
		}

		wLog("set redpen config. width(%d) rgba(%d,%d,%d,%d)->(%f,%f,%f,%f)\n",
				data->penInfo->width, r, g, b, a, c_r, c_g, c_b, c_a);

		gtk_window_get_position(data->annotation_window,
				&data->penInfo->windowPosX, &data->penInfo->windowPosY);

		wLog("set redpen config.  windowPosition:%dx%d\n",
				data->penInfo->windowPosX, data->penInfo->windowPosY);

		//annotate_redpen_draw_test();
	}

	UNLOCK(drawMutex);

}

void calculateVertexes(
		double start_x, double start_y,
		double end_x, double end_y,
		double *x1, double *y1,
		double* x2, double* y2,
		double arrow_length, double arrow_degrees){


	double angle = atan2 (end_y - start_y, end_x - start_x) + M_PI;

	*x1 = end_x + arrow_length * cos(angle - arrow_degrees);
	*y1 = end_y + arrow_length * sin(angle - arrow_degrees);
	*x2 = end_x + arrow_length * cos(angle + arrow_degrees);
	*y2 = end_y + arrow_length * sin(angle + arrow_degrees);
}


void annotate_draw_redpen(PointerEvent evt) {

	LOCK(drawMutex);

	cairo_t * cr = data->annotation_cairo_context;
	if(cr){

		//left:taskbar top:status bar
		int calX, calY;
		calX = evt.x - data->penInfo->windowPosX;
		calY = evt.y - data->penInfo->windowPosY;

		/*
		wLog("draw_redpen_pointer : type[%d] action[%d] coordinates = %d,%d -> %d,%d\n",
				data->penInfo->PenType, evt.action, evt.x, evt.y, calX, calY);
		*/

		evt.x = calX;
		evt.y = calY;

		switch (evt.action) {

		case POINTER_ACTION_DOWN:

			switch (data->penInfo->PenType) {
				case PEN_TYPE_FREELINE:
				case PEN_TYPE_LINE:{
					cairo_move_to(cr, evt.x, evt.y);
					break;
				}
			}

			data->penInfo->downX = evt.x;
			data->penInfo->downY = evt.y;
			break;

		case POINTER_ACTION_MOVE:

			switch (data->penInfo->PenType) {
				case PEN_TYPE_FREELINE:{
					cairo_line_to(cr, evt.x, evt.y);
					cairo_move_to(cr, evt.x, evt.y);
					break;
				}
			}
			break;

		case POINTER_ACTION_UP:

			switch (data->penInfo->PenType) {

				case PEN_TYPE_FREELINE:
				case PEN_TYPE_LINE:{

					cairo_line_to(cr, evt.x, evt.y);
					cairo_stroke(cr);
					break;
				}
				case PEN_TYPE_RECT: {

					int x, y, width, height;
					x = data->penInfo->downX;
					y = data->penInfo->downY;
					width = evt.x - data->penInfo->downX;
					height = evt.y - data->penInfo->downY;

					//wLog("draw rect : %d,%d %dx%d\n", x, y, width, height);

					cairo_rectangle(cr, x, y, width, height);
					cairo_stroke(cr);


					break;
				}
				case PEN_TYPE_CIRCLE: {

					int start_x = data->penInfo->downX;
					int start_y = data->penInfo->downY;
					int width = evt.x - data->penInfo->downX;
					int height = evt.y - data->penInfo->downY;
					double cx = evt.x - (width / 2);
					double cy = evt.y - (height / 2);
					double sx = width/2.;
					double sy = height/2.;

					cairo_save(cr);
					cairo_translate(cr, start_x+width/2., start_y+height/2.);
					cairo_scale(cr, sx, sy);
					cairo_arc(cr, 0., 0., 1., 0., 2.*M_PI);
					cairo_restore(cr);
					cairo_save(cr);
					cairo_stroke(cr);
					cairo_restore(cr);

					//wLog("draw oval : %d,%d %dx%d\n", start_x, start_y, width, height);

					break;
				}
				case PEN_TYPE_ARROWS: {

					double x1;
					double y1;
					double x2;
					double y2;

					int start_x, start_y, end_x, end_y;
					start_x = data->penInfo->downX;
					start_y = data->penInfo->downY;
					end_x = evt.x;
					end_y = evt.y;

					calculateVertexes(start_x, start_y, end_x, end_y, &x1, &y1, &x2, &y2, 15.0, 0.5);

					cairo_move_to(cr, start_x, start_y);
					cairo_line_to(cr, end_x, end_y);
					cairo_move_to(cr, end_x, end_y);
					cairo_line_to (cr, x1, y1);
					cairo_line_to (cr, x2, y2);
					cairo_close_path(cr);
					cairo_stroke_preserve(cr);
					cairo_fill(cr);

					//wLog("draw arrows : %d,%d %dx,%d %dx,%d\n", start_x, start_y, x1, y1, x2, y2);

					break;
				}
			}
			data->penInfo->downX = 0;
			data->penInfo->downY = 0;

			break;
		}
	}

	UNLOCK(drawMutex);

}

/* Release input grab;the input event will be passed below the window. */
void annotate_release_input_grab() {
	ungrab_pointer(gdk_display_get_default());
	//gdk_window_set_cursor (gtk_widget_get_window (data->annotation_window), (GdkCursor *) NULL);
#ifndef _WIN32
	/*
	 * @TODO implement correctly gtk_widget_input_shape_combine_mask
	 * in the quartz gdkwindow or use an equivalent native function;
	 * the current implementation in macosx this does not do nothing.
	 */
	/*
	 * This allows the mouse event to be passed below the transparent annotation;
	 * at the moment this call works only on Linux
	 */
	gtk_widget_input_shape_combine_region(data->annotation_window, NULL);

	const cairo_rectangle_int_t ann_rect = { 0, 0, 0, 0 };
	cairo_region_t *r = cairo_region_create_rectangle(&ann_rect);

	gtk_widget_input_shape_combine_region(data->annotation_window, r);
	cairo_region_destroy(r);

#else
	/*
	 * @TODO WIN32 implement correctly gtk_widget_input_shape_combine_mask
	 * in the win32 gdkwindow or use an equivalent native function.
	 * Now in the gtk implementation the gtk_widget_input_shape_combine_mask
	 * call the gtk_widget_shape_combine_mask that is not the desired behaviour.
	 *
	 */

#endif
}

/* Release the pointer pointer. */
void annotate_release_grab() {
	if (data->is_grabbed) {

		if (data->debug) {
			g_printerr("Release grab\n");
		}

		annotate_release_input_grab();
		gtk_window_present(GTK_WINDOW(get_bar_widget()));
		data->is_grabbed = FALSE;
	}
}

/* Undo reverting to the last save point. */
void annotate_undo() {
	if (data->debug) {
		g_printerr("Undo\n");
	}

	if (data->savepoint_list) {
		if (data->current_save_index
				!= g_slist_length(data->savepoint_list) - 1) {
			data->current_save_index = data->current_save_index + 1;
			annotate_restore_surface();
		}
	}
}

/* Redo to the last save point. */
void annotate_redo() {
	if (data->debug) {
		g_printerr("Redo\n");
	}

	if (data->savepoint_list) {
		if (data->current_save_index != 0) {
			data->current_save_index = data->current_save_index - 1;
			annotate_restore_surface();
		}
	}
}

/* Clear the annotations windows and make an empty savepoint. */
void annotate_clear_screen() {
	if (data->debug) {
		g_printerr("Clear screen\n");
	}

	cairo_new_path(data->annotation_cairo_context);
	clear_cairo_context(data->annotation_cairo_context);
	gtk_widget_queue_draw_area(data->annotation_window, 0, 0,
			gdk_screen_width(), gdk_screen_height());
	/* Add the empty savepoint. */
	annotate_add_savepoint();
}

/* Allocate a invisible cursor that can be used to hide the cursor icon. */
void allocate_invisible_cursor(GdkCursor **cursor) {
	*cursor = gdk_cursor_new(GDK_BLANK_CURSOR);
}

/* Initialize the annotation. */
gint annotate_init(GtkWidget *parent, gchar *iwb_file, gboolean debug) {

	data = g_malloc((gsize) sizeof(AnnotateData));
	gchar* color = g_strdup("FF0000FF");

	/* Initialize the data structure. */
	data->annotation_cairo_context = (cairo_t *) NULL;
	data->savepoint_list = (GSList *) NULL;
	data->current_save_index = 0;
	data->cursor = (GdkCursor *) NULL;
	data->devdatatable = (GHashTable *) NULL;

	data->color = color;
	data->is_grabbed = FALSE;
	data->arrow = FALSE;
	data->rectify = FALSE;
	data->roundify = FALSE;
	data->old_paint_type = ANNOTATE_PEN;

	data->is_cursor_hidden = TRUE;

	data->debug = debug;

	/* Initialize the pen context. */
	data->default_pen = annotate_paint_context_new(ANNOTATE_PEN);
	data->default_eraser = annotate_paint_context_new(ANNOTATE_ERASER);
	data->default_filler = annotate_paint_context_new(ANNOTATE_FILLER);
	data->cur_context = data->default_pen;

	data->is_onload = FALSE;
	data->penInfo = NULL;

	setup_input_devices(data);
	allocate_invisible_cursor(&data->invisible_cursor);

	setup_app(parent);

	return 0;
}

