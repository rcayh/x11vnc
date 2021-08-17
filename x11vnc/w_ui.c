#include <gtk/gtk.h>
#include <time.h>
#include <libgen.h>

#include "x11vnc.h"

#include "w_ui.h"
#include "w_ui_chat.h"
#include "w_utils.h"

#include "libconfig.h"

#ifdef LIB_GTK2
#include <gdk/gdkkeysyms.h>
#endif

#if defined LIB_GTK3 || (defined LIB_GTK2 && defined __GTK_SPINNER_H__)
#define HAS_SPINNER
#endif

#ifdef CLOCK_MONOTONIC_RAW
#define CLOCK_NANOTIME CLOCK_MONOTONIC_RAW
#else
#define CLOCK_NANOTIME CLOCK_MONOTONIC
#endif

#undef GDK_KEY_Return
#define GDK_KEY_Return 0xff0d

#undef GDK_KP_Enter
#define GDK_KP_Enter 0xff8d

bool dist_type_release = 0;

GtkWidget *uwindow;
GtkWidget *vroot;
GtkWidget *vbox;
GtkWidget *vbox2;
GtkWidget *server_ed;
GtkWidget *acceptnum_ed;
GtkWidget *connect_btn;
GtkWidget *status_label;
GtkWidget *status_info;
GtkWidget *spinner;
GtkWidget *no_spinner_label;

gint tmp_pos;

bool main_ui_window_loaded = false;
bool timeLoopEnabled = true;

static char* execdir;
static char iconPath[PATH_MAX];


void removeWidget(GtkWidget *container) {

	size_t count;
	GList *children, *iter;
	children = gtk_container_get_children(GTK_CONTAINER(container));

	for (iter = children; iter != NULL; iter = g_list_next(iter)) {
		gtk_widget_destroy(GTK_WIDGET(iter->data));
		count++;
	}

	wLog("remove children container >>>>> %d\n", count);
	g_list_free(children);
}

void close_grab_func(GtkWidget *widget, gpointer data) {
	gtk_grab_remove(GTK_WIDGET(widget));
}

void close_func(GtkWidget *widget, gpointer data) {
	gtk_widget_destroy(GTK_WIDGET(data));
}

void closing_app_positive_func(GtkWidget *widget, gpointer data) {

	timeLoopEnabled = false;

	if (widget)
		gtk_grab_remove(GTK_WIDGET(widget));

	if (statusNotify != STATUS_NOTIFY_NONE) {
		send_cmd_destroy();
		sleep(1);
	}

	gtk_main_quit();
	exit(-99);
}
void closing_app_negative_func(GtkWidget *widget, gpointer data) {
	close_func(widget, data);
}

gboolean close_app_func(GtkWidget *widget, gpointer data) {

	switch (statusNotify) {

	case STATUS_NOTIFY_NONE:

		closing_app_positive_func(NULL, NULL);
		return FALSE;

	default:
		show_alert_dialog_q(
				"Remote control is connecting. Are you sure you want to quit?",
				closing_app_positive_func, closing_app_negative_func);
		return TRUE;
	}
}

void close_and_shutdown_func(GtkWidget *widget, gpointer data) {
	wLog("close and shutdown.\n");
	close_func(widget, data);
	exit(99);
}

gboolean acceptnum_key_press_callback(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {

	switch(event->keyval){
		case GDK_KEY_Return:
		case GDK_KP_Enter:{

			const gchar *entry_text;
			entry_text = gtk_entry_get_text(GTK_ENTRY(widget));
			wLog("Entry contents: %s\n", entry_text);

			start_remote_control_func(connect_btn, (gpointer) 1);
			break;
		}
	}

	return FALSE;

}
void acceptnum_insert_text_callback(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data)
{
    int i;

    for (i = 0; i < length; i++) {
        if (!isdigit(text[i])) {
            g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
            return;
        }
    }
}

gboolean main_widnow_state_func(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data){

	if(!main_ui_window_loaded){
		gtk_widget_grab_focus(GTK_WIDGET(acceptnum_ed));
		gtk_editable_select_region(GTK_EDITABLE(server_ed), 0, 0);
		main_ui_window_loaded = 1;
	}


	return FALSE;
}

void show_alert_dialog_q(char *message, void (*positiveFunc)(),
		void (*negativeFunc)()) {


	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *dialog_window;

	dialog_window = gtk_dialog_new();
	gtk_window_set_resizable(GTK_WINDOW(dialog_window), FALSE);
	gtk_window_set_position(GTK_WINDOW(dialog_window),
			GTK_WIN_POS_CENTER_ALWAYS);

	GError err;
	gtk_window_set_icon_from_file(dialog_window, iconPath, &err);

	GtkWidget* content_area = NULL;
		GtkWidget* action_area = NULL;
	#ifdef HAS_SPINNER
		content_area = gtk_dialog_get_content_area(dialog_window);
		action_area = gtk_dialog_get_action_area(dialog_window);
	#endif

	g_signal_connect(dialog_window, "destroy", G_CALLBACK(close_grab_func),
			&dialog_window);
	gtk_window_set_title(GTK_WINDOW(dialog_window), "WizHelper-Messages");
	//gtk_container_border_width(GTK_CONTAINER(dialog_window), 5);

	label = gtk_label_new(message);

	gtk_misc_set_padding(GTK_MISC(label), 10, 10);

#ifdef HAS_SPINNER
	gtk_container_add(GTK_CONTAINER(content_area), label);
#else
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->vbox), label, TRUE, TRUE, 0);
#endif

	gtk_widget_show(label);

	button = gtk_button_new_with_label("Yes");
	g_signal_connect(button, "clicked", G_CALLBACK(positiveFunc),
			dialog_window);
	//GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
#ifdef HAS_SPINNER
	gtk_box_pack_start(GTK_BOX(action_area), button, TRUE, TRUE, 0);
#else
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->action_area), button, TRUE, TRUE, 0);
#endif
	gtk_widget_show(button);

	button = gtk_button_new_with_label("No");
	g_signal_connect(button, "clicked", G_CALLBACK(negativeFunc),
			dialog_window);
	//GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
#ifdef HAS_SPINNER
	gtk_box_pack_start(GTK_BOX(action_area), button, TRUE, TRUE, 0);
#else
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->action_area), button, TRUE, TRUE, 0);
#endif
	gtk_widget_grab_default(button);
	gtk_widget_show(button);

	gtk_widget_show(dialog_window);
	gtk_grab_add(dialog_window);
}

void show_alert_dialog(char *message, bool shutdown) {

	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *dialog_window;

	dialog_window = gtk_dialog_new();
	gtk_window_set_resizable(GTK_WINDOW(dialog_window), FALSE);
	gtk_window_set_position(GTK_WINDOW(dialog_window),
			GTK_WIN_POS_CENTER_ALWAYS);

	GError err;
	gtk_window_set_icon_from_file(dialog_window, iconPath, &err);

	GtkWidget* content_area = NULL;
	GtkWidget* action_area = NULL;
#ifdef HAS_SPINNER
	content_area = gtk_dialog_get_content_area(dialog_window);
	action_area = gtk_dialog_get_action_area(dialog_window);
#endif

	void* active_func;
	if (shutdown)
		active_func = G_CALLBACK(close_and_shutdown_func);
	else
		active_func = G_CALLBACK(close_func);

	g_signal_connect(dialog_window, "destroy", G_CALLBACK(close_grab_func),
			&dialog_window);
	gtk_window_set_title(GTK_WINDOW(dialog_window), "WizHelper-Messages");
	//gtk_container_border_width(GTK_CONTAINER(dialog_window), 5);

	label = gtk_label_new(message);
	gtk_misc_set_padding(GTK_MISC(label), 10, 10);

#ifdef HAS_SPINNER
	gtk_container_add(GTK_CONTAINER(content_area), label);
#else
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->vbox), label, TRUE, TRUE, 0);
#endif

	gtk_widget_show(label);

	button = gtk_button_new_with_label("Ok");
	g_signal_connect(button, "clicked", active_func, dialog_window);
	//GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

#ifdef HAS_SPINNER
	gtk_box_pack_start(GTK_BOX(action_area), button, TRUE, TRUE, 0);
#else
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->action_area), button, TRUE, TRUE, 0);
#endif
	//gtk_widget_grab_default(button);
	gtk_widget_show(button);

	gtk_widget_show(dialog_window);
	gtk_grab_add(dialog_window);
}

void start_remote_control_func(GtkWidget *widget, gpointer data) {

	bool success;

	const gchar* serverData = gtk_entry_get_text(GTK_ENTRY(server_ed));
	const gchar* acceptumData = gtk_entry_get_text(GTK_ENTRY(acceptnum_ed));

	wLog("serverUri[%s] acceptNum[%s]\n", serverData, acceptumData);

	if (strlen(serverData) <= 0) {
		show_alert_dialog("please enter server uri", false);
		return;
	}

	int scheme_pos = -1;
	int host_pos = -1;
	int port = 0;

	char* scheme;
	char thost[100];
	char host[100];

	char page[100];

	success = startsWith("https://", serverData);
	if (success) {
		scheme = "https";
		scheme_pos = 8;
		port = 443;
		sscanf(serverData, "https://%99[^:]:%99d/%99[^\n]", thost, &port, page);
	} else {
		success = startsWith("http://", serverData);
		if (!success) {
			show_alert_dialog(
					"invalid server uri :\n\nex) http://192.168.0.1:80 or https://domain.com",
					false);
			return;
		}

		scheme = "http";
		scheme_pos = 7;
		port = 80;
		sscanf(serverData, "http://%99[^:]:%99d/%99[^\n]", thost, &port, page);
	}

	wLog("[uri] scheme=[%s] host=[%s] port=[%d] page=[%s]\n", scheme, thost,
			port, page);

	const char *host_ptr1 = strchr(thost, '/');
	if (host_ptr1) {
		host_pos = host_ptr1 - thost;
		memcpy(host, thost, host_pos);
	} else {
		const char *host_ptr2 = strchr(thost, '/');
		if (host_ptr2) {
			host_pos = host_ptr2 - thost;
			memcpy(host, thost, host_pos);
		}
	}

	if (host_pos == -1) {
		strcpy(host, thost);
	} else {
		memset(&page[0], 0, sizeof(page));
		memcpy(page, thost + host_pos, strlen(thost) - host_pos);
		wLog("[uri] replace host = %s =====> %s  p[%s]\n", thost, host,
				page);
	}

	success = isRegexMatches(REGEX_IPADDRESS, host);
	if (!success) {

		success = isRegexMatches(REGEX_DOMAIN, host);
		if (!success) {
			show_alert_dialog(
					"invalid server uri : required ip address or domain\n\nex) http://192.168.0.1:80 or https://domain.com",
					false);
			return;
		}
	}

	if (strlen(acceptumData) != 4) {gtk_editable_select_region(GTK_EDITABLE(server_ed), 0, 0);
		show_alert_dialog("please enter accept number.", false);
		return;
	}

	if (!isDigit(acceptumData)) {
		show_alert_dialog("please enter numbers.", false);
		return;
	}

	free(waitInfo);
	waitInfo = (ControlWaitInfoPtr*) malloc(sizeof(ControlWaitInfo));

	strcpy(waitInfo->webServerScheme, scheme);
	strcpy(waitInfo->webServerHost, host);
	waitInfo->webServerPort = port;

#ifdef REQUEST_TEST_CONTROL_LIST
	strcpy(waitInfo->acceptNum, "test");
#else
	strcpy(waitInfo->acceptNum, acceptumData);
#endif

	gtk_widget_hide(vbox);

#ifdef HAS_SPINNER
	gtk_widget_show(spinner);
#endif
	gtk_widget_show(vbox2);


	/* save server uri */
	/*
	char confPath[PATH_MAX];
	sprintf(confPath, "%s/%s", execdir, "../res/w_conf.properties");
	wLog("conf path = %s\n", confPath);

	config_t cfg;
	config_init(&cfg);

	if(!config_read_file(&cfg, confPath)){
		wLog("properties load error : %s:%d - %s\n",
				config_error_file(&cfg),
				config_error_line(&cfg),
				config_error_text(&cfg));
	}
	else{
		config_setting_t* default_server_uri;
		default_server_uri = config_lookup(&cfg, "default_server_uri");

		if(default_server_uri != NULL){
			config_setting_set_string(default_server_uri, serverData);
		}
		config_write_file(&cfg, confPath);
	}
	config_destroy(&cfg);
	*/

	pthread_t thread;
	pthread_create(&thread, NULL, start_main, NULL);

}

int ui_main(int argc, char* argv[]) {

	if (!g_thread_supported())
		g_thread_init(NULL);

	gdk_threads_init();
	gdk_threads_enter();

	XInitThreads();

	gtk_init(&argc, &argv);


#ifdef DIST_TYPE_RELEASE
	dist_type_release = true;
#else
	dist_type_release = false;
#endif

	guint gtk_compile_major_ver;
	guint gtk_current_major_ver;
	guint gtk_current_minor_ver;

#ifdef LIB_GTK2
	gtk_compile_major_ver = 2;
	gtk_current_major_ver = gtk_major_version;
	gtk_current_minor_ver = gtk_minor_version;
#else
	gtk_compile_major_ver = gtk_get_major_version();
	gtk_current_major_ver = gtk_get_major_version();
	gtk_current_minor_ver = gtk_get_minor_version();
#endif


	wLog("startup. compile gtk version=[v.%d]  current gtk version[v.%d.%d]\n",
			gtk_compile_major_ver,
			gtk_current_major_ver,
			gtk_current_minor_ver);

	if(0){

#ifdef LIB_GTK2
#else
		if(!chat_is_create_window()) create_chat_window(iconPath, NULL);
		chat_insert_message(CHAT_USER_TYPE_ADVISER, "test");
		gtk_main();
		gdk_threads_leave();
#endif
		return;
	}

	uwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(uwindow), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_title(GTK_WINDOW(uwindow), "WizHelper");
	gtk_widget_set_size_request(uwindow, 500, 200);
	gtk_window_set_resizable(GTK_WINDOW(uwindow), FALSE);


	char execPath[PATH_MAX];
	getExecPath(execPath);
	execdir = dirname(execPath);

	sprintf(iconPath, "%s/%s", execdir, "../res/w_icon.png");
	wLog("icon path = %s\n", iconPath);

	GError err;
	gtk_window_set_icon_from_file(GTK_WINDOW(uwindow), iconPath, &err);

	g_signal_connect(uwindow, "window_state_event", G_CALLBACK(main_widnow_state_func), NULL);
	g_signal_connect(uwindow, "delete_event", G_CALLBACK(close_app_func), NULL);

	gtk_container_set_border_width(GTK_CONTAINER(uwindow), 20);

	vroot = gtk_vbox_new(TRUE, 5);

	/** VBOX **/
	vbox = gtk_vbox_new(TRUE, 5);

	GtkWidget *tmp_server_label = gtk_label_new("Please enter server URI");
	gtk_misc_set_alignment(GTK_MISC(tmp_server_label), 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), tmp_server_label, TRUE, TRUE, 0);
	gtk_widget_show(tmp_server_label);

	server_ed = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(server_ed), 255);

	/**
	 * SERVER URI
	 */
	char defaulServerUri[256];
	char confPath[PATH_MAX];
	sprintf(confPath, "%s/%s", execdir, "../res/w_conf.properties");
	wLog("conf path = %s\n", confPath);
	config_t cfg;

	config_init(&cfg);
	if(!config_read_file(&cfg, confPath)){
		wLog("properties load error : %s:%d - %s\n",
				config_error_file(&cfg),
	            config_error_line(&cfg),
				config_error_text(&cfg));
	}
	else{
		config_setting_t* default_server_uri;
		default_server_uri = config_lookup(&cfg, "default_server_uri");

		if(default_server_uri != NULL){
			strcpy(defaulServerUri, config_setting_get_string(default_server_uri));
		}
	}
	config_destroy(&cfg);

	if(strlen(defaulServerUri) == 0){
		strcpy(defaulServerUri, DEFAULT_HTTP_URI);
	}
	gtk_entry_set_text(GTK_ENTRY(server_ed), defaulServerUri);



	//append text
	/*
	 tmp_pos = GTK_ENTRY(entry)->text_length;
	 gtk_editable_insert_text(GTK_EDITABLE(entry), " world", -1, &tmp_pos);
	 */

	//gtk_editable_select_region(GTK_EDITABLE(server_ed), 0, GTK_ENTRY(server_ed)->text_length);
	gtk_box_pack_start(GTK_BOX(vbox), server_ed, TRUE, TRUE, 0);
	gtk_widget_show(server_ed);

	GtkWidget *tmp_acceptnum_label = gtk_label_new("Please enter Accept Number");
	gtk_misc_set_alignment(GTK_MISC(tmp_acceptnum_label), 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), tmp_acceptnum_label, TRUE, TRUE, 0);
	gtk_widget_show(tmp_acceptnum_label);

	acceptnum_ed = gtk_entry_new();

	gtk_entry_set_max_length(GTK_ENTRY(acceptnum_ed), 4);
	g_signal_connect(acceptnum_ed, "key-press-event", G_CALLBACK(acceptnum_key_press_callback), acceptnum_ed);
	g_signal_connect(acceptnum_ed, "insert-text", G_CALLBACK(acceptnum_insert_text_callback), NULL);
#ifdef REQUEST_TEST_CONTROL_LIST
	gtk_entry_set_text(GTK_ENTRY(acceptnum_ed), "2222");
#endif

	//gtk_editable_select_region(GTK_EDITABLE(acceptnum_ed), 0, GTK_ENTRY(acceptnum_ed)->text_length);
	gtk_box_pack_start(GTK_BOX(vbox), acceptnum_ed, TRUE, TRUE, 0);
	gtk_widget_show(acceptnum_ed);

	connect_btn = gtk_button_new_with_label("Start Remote Control");
	g_signal_connect(connect_btn, "clicked",
			G_CALLBACK(start_remote_control_func), (gpointer) 1);
	gtk_box_pack_start(GTK_BOX(vbox), connect_btn, TRUE, TRUE, 0);
	gtk_widget_show(connect_btn);

	gtk_widget_show(vbox);

	gtk_box_pack_start(GTK_BOX(vroot), vbox, TRUE, TRUE, 0);

	//gtk_container_add(GTK_CONTAINER(window), vbox);

	/** VBOX2 **/
	vbox2 = gtk_vbox_new(TRUE, 5);

	status_label = gtk_label_new("");
	gtk_label_set_justify(GTK_LABEL(status_label), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox2), status_label, TRUE, TRUE, 0);
	gtk_widget_show(status_label);

	status_info = gtk_label_new("");
	gtk_label_set_justify(GTK_LABEL(status_info), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox2), status_info, TRUE, TRUE, 0);
	gtk_widget_show(status_info);


#ifdef HAS_SPINNER
	spinner = gtk_spinner_new();
	gtk_spinner_start(GTK_SPINNER(spinner));
	gtk_box_pack_start(GTK_BOX(vbox2), spinner, TRUE, TRUE, 0);
	gtk_widget_show(spinner);
#else
	no_spinner_label = gtk_label_new("Proceeding ...");
	gtk_box_pack_start(GTK_BOX(vbox2), no_spinner_label, TRUE, TRUE, 0);
	gtk_widget_show(no_spinner_label);
#endif

	gtk_box_pack_start(GTK_BOX(vbox2), spinner, TRUE, TRUE, 0);

	//gtk_widget_show(vbox2);

	gtk_box_pack_start(GTK_BOX(vroot), vbox2, TRUE, TRUE, 0);

	gtk_widget_show(vroot);
	gtk_container_add(GTK_CONTAINER(uwindow), vroot);

	gtk_widget_show(uwindow);

	gtk_main();

#ifdef HAS_SPINNER
	GdkEvent *fevent = gdk_event_new(GDK_FOCUS_CHANGE);
	gtk_widget_send_focus_change(GTK_WIDGET(acceptnum_ed), fevent);
#endif

	gtk_widget_show(status_label);

	gdk_threads_leave();
	return (0);
}

void* ui_time_loop(void* ptr) {

	struct timespec start, end;
	clock_gettime(CLOCK_NANOTIME, &start);

	uint64_t count = 0;
	char runningTime[9];

	while (timeLoopEnabled) {

		count++;
		clock_gettime(CLOCK_NANOTIME, &end);

		millisToTimeFormat(start, end, runningTime);
		//wLog("time_loop >> %s\n", runningTime);

		gdk_threads_enter();
		gtk_label_set_label(GTK_LABEL(status_info), runningTime);
		gdk_threads_leave();

		sleep(1);

	}
}

/**
 * Called multithread
 */
void ui_set_status_text(const char* message) {

	LOCK(uiMutex);

	wLog("ui_set_status_text >> %s\n", message);
	wLog("\n");

	gdk_threads_enter();
	gtk_label_set_label(GTK_LABEL(status_label), message);
	gdk_threads_leave();

	UNLOCK(uiMutex);
}
void ui_finish(char* message) {

	LOCK(uiMutex);

	wLog("ui_finish >> %s\n", message);
	wLog("\n");

	gdk_threads_enter();
	gtk_label_set_label(GTK_LABEL(status_label), message);
	gtk_widget_hide(spinner);
	show_alert_dialog(message, true);
	gdk_threads_leave();

	UNLOCK(uiMutex);
}
void ui_resume(char* message) {

	LOCK(uiMutex);

	wLog("ui_stop >> %s\n", message);
	wLog("\n");

	statusNotify = STATUS_NOTIFY_NONE;

	gdk_threads_enter();
	gtk_label_set_label(GTK_LABEL(status_label), message);
	gtk_widget_hide(vbox2);
	gtk_widget_show(vbox);
	show_alert_dialog(message, false);
	gdk_threads_leave();

	UNLOCK(uiMutex);
}

void ui_connected() {

	LOCK(uiMutex);

	wLog("ui_connected >> ");
	wLog("\n");

	gdk_threads_enter();
	gtk_widget_hide(spinner);
	gtk_widget_show(status_info);
	//gtk_window_iconify(GTK_WINDOW(uwindow));
	gdk_threads_leave();

	pthread_t thread;
	pthread_create(&thread, NULL, ui_time_loop, NULL);

	UNLOCK(uiMutex);

}

void ui_chat_receive_message(const char* message){

#ifdef LIB_GTK2
#else
	gdk_threads_enter();

	if(!chat_is_create_window()) create_chat_window(iconPath, message);
	chat_insert_message(CHAT_USER_TYPE_ADVISER, message);

	gdk_threads_leave();
#endif
}

