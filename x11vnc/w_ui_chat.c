
#include <gdk/gdk.h>
#include <gtk/gtk.h>


#include "w_ui_chat.h"

#ifdef LIB_GTK2
#include <gdk/gdkkeysyms.h>
#else
#endif

static GtkBuilder *chat_gtk_builder = NULL;
static GtkWidget *chat_window;
static GtkWidget *chat_sc_window;
static GtkWidget *chat_send_btn;
static GtkWidget *chat_grid;
static GtkWidget *chat_input;
static char *chat_first_message;

#ifdef LIB_GTK2
#else
static GdkRGBA chat_adviser_color;
static GdkRGBA chat_none_color;
#endif



static bool chat_onload = false;
static bool chat_message_count = false;
static bool chat_is_iconify = false;

void chat_update_ui(){

	rfbLog("chat_update_ui\n");

	gtk_container_set_border_width(GTK_CONTAINER(chat_window), 5);

	chat_grid = GTK_WIDGET(gtk_builder_get_object(chat_gtk_builder, "chat_grid"));

	gtk_grid_set_row_homogeneous(chat_grid, TRUE);
	gtk_grid_set_column_homogeneous(chat_grid, TRUE);

	gtk_grid_insert_column(chat_grid, 0);

	chat_send_btn = GTK_WIDGET(gtk_builder_get_object(chat_gtk_builder, "send_btn"));
	chat_input = GTK_WIDGET(gtk_builder_get_object(chat_gtk_builder, "input"));

	chat_sc_window = GTK_WIDGET(gtk_builder_get_object(chat_gtk_builder, "sc_window"));
	gtk_misc_set_padding(GTK_MISC(chat_sc_window), 10, 10);

#ifdef LIB_GTK2
#else
	gdk_rgba_parse(&chat_adviser_color, "#ff0000");
	gdk_rgba_parse(&chat_none_color, "#0000ff");
#endif

	if(chat_first_message != NULL){
		chat_insert_message(CHAT_USER_TYPE_ADVISER, chat_first_message);
		free(chat_first_message);
		chat_first_message = NULL;
	}
}

G_MODULE_EXPORT void
on_chat_send_btn_click_event(GtkToolButton *toolbutton, gpointer func_data){
	rfbLog("on_chat_send_btn_click_event!\n");

	const gchar* message = gtk_entry_get_text(GTK_ENTRY(chat_input));

	if (strlen(message) <= 0) {
		return;
	}

	chat_insert_message(CHAT_USER_TYPE_NONE, message);
	send_chat_message(message);
	gtk_entry_set_text(GTK_ENTRY(chat_input), "");
}


G_MODULE_EXPORT gboolean
on_chat_input_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data){
	//rfbLog("on_chat_input_key_press_event! == %d\n", event->keyval);

	if(event->keyval == GDK_KEY_Return)
		on_chat_send_btn_click_event(NULL, NULL);

	return FALSE;
}


G_MODULE_EXPORT gboolean
on_chat_window_delete_event(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data){
	rfbLog("on_chat_window_delete_event!\n");
	chat_destroy();
}

G_MODULE_EXPORT gboolean
on_chat_window_state_event(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data){
	rfbLog("on_chat_window_state_event! = %d\n", event->type);

	if(!chat_onload){
		chat_update_ui();
		chat_onload = true;
	}

	if(event->new_window_state & GDK_WINDOW_STATE_ICONIFIED){
		chat_is_iconify = true;
	}
}

G_MODULE_EXPORT gboolean
on_chat_window_configure_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data){
	rfbLog("on_chat_window_configure_event!\n");
}


void create_chat_window(const char* iconPath, char* firstMessage){

	rfbLog("create chat window. start\n");

	chat_first_message = firstMessage;
	chat_gtk_builder = gtk_builder_new();

	GError* error = (GError *) NULL;
	gtk_builder_add_from_string(chat_gtk_builder, CHAT_UI_TEXT, strlen(CHAT_UI_TEXT), error);
	if (error) {
		g_warning("Failed to load builder file: %s", error->message);
		//g_error_free(error);
		return;
	}

	rfbLog("1\n");

	chat_window = GTK_WIDGET(gtk_builder_get_object(chat_gtk_builder, "chat_window"));
	gtk_window_set_title(GTK_WINDOW(chat_window), "WizHelper-Chat");
	gtk_builder_connect_signals(chat_gtk_builder, (gpointer) NULL);
	gtk_window_set_deletable(GTK_WINDOW(chat_window), FALSE);

	GError err;
	gtk_window_set_icon_from_file(chat_window, iconPath, &err);

	gtk_widget_show_all(chat_window);

	//gtk_main();
	rfbLog("create chat window. finish\n");

}


void chat_insert_message(const int userType, const char* message){

	rfbLog("chat insert message\n");

	char msg[2048];
	sprintf(msg, "%s: %s", (userType==CHAT_USER_TYPE_ADVISER?"Adviser":"My"), message);

	GtkWidget *label1 = gtk_label_new(msg);
	gtk_misc_set_padding(GTK_MISC(label1), 10, 10);

	gtk_label_set_use_markup(GTK_LABEL(label1), TRUE);
	//gtk_misc_set_padding(GTK_MISC(label1), 10, 10);

	gtk_grid_insert_row(chat_grid, chat_message_count);

	gtk_grid_attach(chat_grid, label1, 0, chat_message_count, 1, 1);

	gtk_widget_show(label1);

	if(userType == CHAT_USER_TYPE_ADVISER){
#ifdef LIB_GTK3
		//gtk_label_set_xalign(GTK_LABEL(label1), 0.1);
		gtk_widget_override_color(label1, GTK_STATE_FLAG_NORMAL, &chat_adviser_color);
#endif

	}
	else{
#ifdef LIB_GTK3
		//gtk_label_set_xalign(GTK_LABEL(label1), 1.0);
		gtk_widget_override_color(label1, GTK_STATE_FLAG_NORMAL, &chat_none_color);
#endif
	}

	gtk_label_set_max_width_chars(GTK_LABEL(label1), 30);
	gtk_label_set_line_wrap(GTK_LABEL(label1), TRUE);

	chat_message_count++;


	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(chat_sc_window);
	gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));

	if(chat_is_iconify){
		gtk_window_deiconify(chat_window);
		chat_is_iconify = false;
	}
}

bool chat_is_create_window(){
	return chat_onload;
}

void chat_destroy(){

	if(chat_gtk_builder){
		g_object_unref(chat_gtk_builder);
		chat_gtk_builder = NULL;
	}

	chat_onload = 0;
	chat_message_count = 0;

}
