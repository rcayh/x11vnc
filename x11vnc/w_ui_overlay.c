
#include <stdlib.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "w_ui_overlay.h"

static gboolean overlayNotSupportAlert = True;
static gboolean overlayLoaded = False;

int overlay_pointer_event_message(PointerEvent evt){

#ifdef LIB_GTK2
#else

	if(!overlayLoaded){
		wLog("overlay canvas is not loaded.\n");
		return -1;
	}

	annotate_draw_redpen(evt);
#endif

	return 1;
}


int overlay_create(RedpenInfo penInfo) {

#ifdef LIB_GTK2
#else
	wLog("overlay create >>>>>>>>>>>>>>> \n");

	int major_opcode, first_event, first_error;

	Display *display;
	display = XOpenDisplay(NULL);

	if(!XQueryExtension(display, "Composite", &major_opcode, &first_event, &first_error)){
		wLog("no compisite manager.\n");
		if(overlayNotSupportAlert && penInfo.onoff == 1){
			overlayNotSupportAlert = False;
			gdk_threads_enter();
			show_alert_dialog("Not support Composite with current display.", False);
			gdk_threads_leave();
		}

	    return -1;
	}

	if(overlayLoaded){
		annotate_set_redpen_info(penInfo);
		annotate_redpen_config();
	}
	else{

		overlayLoaded = True;

		//g_printf("overlay_create >>>>>>> %d\n", penInfo.onoff);

		GtkWidget *background_window = (GtkWidget *) NULL;
		GtkWidget *annotation_window = (GtkWidget *) NULL;
		GtkWidget *ardesia_bar_window = (GtkWidget *) NULL;

		background_window = create_background_window();

		if (background_window == NULL) {
			wLog("error >>> b window create failed.\n");
			overlay_destroy();
			return -1;
		}

		gtk_widget_show(background_window);

		set_background_window(background_window);

		/* Initialize the annotation window. */
		annotate_init(background_window, NULL, True);

		annotation_window = get_annotation_window();

		if (annotation_window == NULL) {
			wLog("error >>> a window create failed.\n");
			overlay_destroy();
			return -1;
		}

		/* Postcondition: the annotation window is valid. */
		gtk_window_set_keep_above(GTK_WINDOW(annotation_window), TRUE);

		gtk_widget_show(annotation_window);

		ardesia_bar_window = create_bar_window(annotation_window);

		if (ardesia_bar_window == NULL) {
			wLog("error >>> d window create failed.\n");
			overlay_destroy();
			return -1;
		}

		gtk_window_set_keep_above(GTK_WINDOW(ardesia_bar_window), TRUE);
		gtk_widget_show(ardesia_bar_window);
		//*/

		annotate_set_redpen_info(penInfo);
	}
#endif
	return 1;
}

int overlay_destroy() {

#ifdef LIB_GTK2
#else
	wLog("overlay destroy >>>>>>>>>>>>>>> \n");

	if(overlayLoaded){
		destroy_background_window();
		destroy_annotation_window();
		destroy_bar_window();
		overlayLoaded = False;
	}
	overlayNotSupportAlert = True;
#endif
	return 1;
}

