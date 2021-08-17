#yh
STATIC_LIBS_PATH=/usr/local/src/static_libs
#DIST_TYPE_RELEASE=-DLIBVNCSERVER_DIST_TYPE_RELEASE 
W_CONF_PATH=../res/nts/2/w_conf.properties

#gtk2
#GTK_VERSION=gtk2
#LIB_GTK_BUILD_EV=-DLIBVNCSERVER_GTK2 -DLIBVNCSERVER_HAVE_X11 -DLIBVNCSERVER_HAVE_XTEST
#LIB_GTK_INCLUDES=-I/usr/include/gtk-2.0 `pkg-config --cflags --libs gtk+-2.0`
#LIB_GTK_SOURCES=
#LIB_GTK_LD_SHARED_LIBS=-lgtk-x11-2.0 -lgdk-x11-2.0

#gtk3+
GTK_VERSION=gtk3
LIB_GTK_BUILD_EV=-DLIBVNCSERVER_GTK3 -DLIBVNCSERVER_HAVE_X11 -DLIBVNCSERVER_HAVE_XTEST -DNE_DEBUG
LIB_GTK_INCLUDES=-I/usr/include/gtk-3.0 `pkg-config --cflags --libs gtk+-3.0`
LIB_GTK_SOURCES=w_ui_overlay_annotation_window_callbacks.c w_ui_overlay_background_window_callbacks.c w_ui_overlay_annotation_window.c w_ui_overlay_background_window.c w_ui_overlay_utils.c w_ui_overlay_input.c w_ui_overlay_bar_callbacks.c w_ui_overlay_bar.c w_ui_chat.c
LIB_GTK_LD_SHARED_LIBS=-lgtk-3 -lgdk-3

CUSTOM_INCLUDES = $(LIB_GTK_BUILD_EV) $(DIST_TYPE_RELEASE) -I$(STATIC_LIBS_PATH)/libneon/include/neon -I$(STATIC_LIBS_PATH)/libcjson/include/cjson -I$(STATIC_LIBS_PATH)/libuuid/include -I$(STATIC_LIBS_PATH)/libconfig/include $(LIB_GTK_INCLUDES)
CUSTOM_SOURCES = $(LIB_GTK_SOURCES) w_common.c w_ui.c w_ui_overlay.c w_relay.c w_http.c w_utils.c wizhelper.c  
CUSTOM_LD_SHARED_LIBS = $(LIB_GTK_LD_SHARED_LIBS) -lcairo -lgdk_pixbuf-2.0 -lgobject-2.0 -lglib-2.0 -lgthread-2.0 -lm  
CUSTOM_LD_STATIC_LIBS = $(STATIC_LIBS_PATH)/libcjson/lib/libcjson.a $(STATIC_LIBS_PATH)/libcjson/lib/libcjson_utils.a $(STATIC_LIBS_PATH)/libneon/lib/libneon.a $(STATIC_LIBS_PATH)/libuuid/lib/libuuid.a $(STATIC_LIBS_PATH)/libconfig/lib/libconfig.a
CFLAGS = -rdynamic -lssl -lcrypto
#pointer : -lXtst

 

if HAVE_SYSTEM_LIBVNCSERVER
LDADD = @SYSTEM_LIBVNCSERVER_LIBS@ @WSOCKLIB@
else
LDADD = ../libvncserver/libvncserver.a ../libvncclient/libvncclient.a @WSOCKLIB@
endif

if OSX
FRAMEWORKS = -framework ApplicationServices -framework Carbon -framework IOKit -framework Cocoa
if OSX_OPENGL
FRAMEWORKS += -framework OpenGL
if HAVE_X11
GL = /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib
FRAMEWORKS += -dylib_file $(GL):$(GL)
endif
endif
wizhelper_LDFLAGS = $(FRAMEWORKS)
wizhelper_CFLAGS = -ObjC
endif

DIST_SUBDIRS = misc

#desktopdir = $(datadir)/applications
#desktop_DATA = x11vnc.desktop

resdir = ${prefix}/res
res_DATA = ../res/w_icon.png $(W_CONF_PATH)

homedir = ${prefix}
home_DATA = w_startup.sh w_install.sh w_setup.desktop

ARCH := $(shell arch)
#LONGBIT := $(shell getconf LONG_BIT)

install-data-hook:
	chmod 755 ${prefix}/w_startup.sh
	chmod 755 ${prefix}/w_install.sh
	
	mv ${prefix}/w_setup.desktop ${prefix}/../
	chmod 755 ${prefix}/../w_setup.desktop
	
	makeself --notemp /app/wizhelper /app/setup_wizhelper.run "" ./w_install.sh
	
	cd /app && tar cvfz setup_wizhelper_$(ARCH)_$(GTK_VERSION).tar.gz w_setup.desktop setup_wizhelper.run 
	
	
libsdir = ${prefix}/libs
libs_DATA = $(STATIC_LIBS_PATH)/libjpeg/lib/libjpeg.so $(STATIC_LIBS_PATH)/libjpeg/lib/libjpeg.so.8


#man_MANS=x11vnc.1
EXTRA_DIST=ChangeLog README RELEASE-NOTES tkx11vnc $(man_MANS) $(desktop_DATA) $(home_DATA) $(res_DATA)

if CYGIPC
LD_CYGIPC=-lcygipc
endif

bin_PROGRAMS=wizhelper
wizhelper_SOURCES = $(CUSTOM_SOURCES) 8to24.c appshare.c avahi.c cleanup.c connections.c cursor.c gui.c help.c inet.c keyboard.c linuxfb.c macosx.c macosxCG.c macosxCGP.c macosxCGS.c macosx_opengl.c options.c pm.c pointer.c rates.c remote.c scan.c screen.c selection.c solid.c sslcmds.c sslhelper.c uinput.c unixpw.c user.c userinput.c util.c v4l.c win_utils.c x11vnc.c x11vnc_defs.c xdamage.c xevents.c xinerama.c xkb_bell.c xrandr.c xrecord.c xwrappers.c 8to24.h allowed_input_t.h avahi.h blackout_t.h cleanup.h connections.h cursor.h enc.h enums.h gui.h help.h inet.h keyboard.h linuxfb.h macosx.h macosxCG.h macosxCGP.h macosxCGS.h macosx_opengl.h nox11.h nox11_funcs.h options.h params.h pm.h pointer.h rates.h remote.h scan.h screen.h scrollevent_t.h selection.h solid.h sslcmds.h sslhelper.h ssltools.h tkx11vnc.h uinput.h unixpw.h user.h userinput.h util.h v4l.h win_utils.h winattr_t.h x11vnc.h xdamage.h xevents.h xinerama.h xkb_bell.h xrandr.h xrecord.h xwrappers.h 

if HAVE_SYSTEM_LIBVNCSERVER
INCLUDES_LIBVNCSERVER = @SYSTEM_LIBVNCSERVER_CFLAGS@
else
INCLUDES_LIBVNCSERVER =
endif
INCLUDES = $(INCLUDES_LIBVNCSERVER) $(CUSTOM_INCLUDES) @X_CFLAGS@ @AVAHI_CFLAGS@

wizhelper_LDADD=$(LDADD) @SSL_LIBS@ @CRYPT_LIBS@ @X_LIBS@ @AVAHI_LIBS@ $(CUSTOM_LD_STATIC_LIBS) $(CUSTOM_LD_SHARED_LIBS) $(LD_CYGIPC)
