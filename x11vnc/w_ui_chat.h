

#ifndef W_UI_CHAT_H_
#define W_UI_CHAT_H_

#include "rfb/rfb.h"

#include "w_common.h"

#define CHAT_USER_TYPE_ADVISER 1
#define CHAT_USER_TYPE_NONE 2


bool chat_is_create_window();
void chat_insert_message(const int userType, const char* message);


#define CHAT_UI_TEXT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>                                                        "\
		"<interface>                                                                                                    "\
		"  <object class=\"GtkWindow\" id=\"chat_window\">                                                              "\
		"    <property name=\"can_focus\">False</property>                                                              "\
		"    <property name=\"window_position\">center-always</property>                                                "\
		"    <property name=\"default_width\">400</property>                                                            "\
		"    <property name=\"default_height\">600</property>                                                           "\
		"    <signal name=\"window-state-event\" handler=\"on_chat_window_state_event\" swapped=\"no\"/>                "\
		"    <signal name=\"configure-event\" handler=\"on_chat_window_configure_event\" swapped=\"no\"/>               "\
		"    <signal name=\"delete-event\" handler=\"on_chat_window_delete_event\" swapped=\"no\"/>                     "\
		"    <child>                                                                                                    "\
		"      <object class=\"GtkBox\" id=\"box1\">                                                                    "\
		"        <property name=\"visible\">True</property>                                                             "\
		"        <property name=\"can_focus\">False</property>                                                          "\
		"        <property name=\"orientation\">vertical</property>                                                     "\
		"        <child>                                                                                                "\
		"          <object class=\"GtkScrolledWindow\" id=\"sc_window\">                                                "\
		"            <property name=\"visible\">True</property>                                                         "\
		"            <property name=\"can_focus\">True</property>                                                       "\
		"            <property name=\"shadow_type\">in</property>                                                       "\
		"            <child>                                                                                            "\
		"              <object class=\"GtkViewport\" id=\"viewport1\">                                                  "\
		"                <property name=\"visible\">True</property>                                                     "\
		"                <property name=\"can_focus\">False</property>                                                  "\
		"                <child>                                                                                        "\
		"                  <object class=\"GtkGrid\" id=\"chat_grid\">                                                  "\
		"                    <property name=\"visible\">True</property>                                                 "\
		"                    <property name=\"can_focus\">False</property>                                              "\
		"                    <property name=\"row_spacing\">10</property>                                               "\
		"                    <property name=\"column_spacing\">10</property>                                            "\
		"                    <child>                                                                                    "\
		"                      <placeholder/>                                                                           "\
		"                    </child>                                                                                   "\
		"                  </object>                                                                                    "\
		"                </child>                                                                                       "\
		"              </object>                                                                                        "\
		"            </child>                                                                                           "\
		"          </object>                                                                                            "\
		"          <packing>                                                                                            "\
		"            <property name=\"expand\">True</property>                                                          "\
		"            <property name=\"fill\">True</property>                                                            "\
		"            <property name=\"padding\">5</property>                                                            "\
		"            <property name=\"position\">0</property>                                                           "\
		"          </packing>                                                                                           "\
		"        </child>                                                                                               "\
		"        <child>                                                                                                "\
		"          <object class=\"GtkBox\" id=\"box2\">                                                                "\
		"            <property name=\"visible\">True</property>                                                         "\
		"            <property name=\"can_focus\">False</property>                                                      "\
		"            <child>                                                                                            "\
		"              <object class=\"GtkEntry\" id=\"input\">                                                         "\
		"                <property name=\"visible\">True</property>                                                     "\
		"                <property name=\"can_focus\">True</property>                                                   "\
		"                <property name=\"invisible_char\">¡Ü</property>                                                "\
		"				 <signal name=\"key-press-event\" handler=\"on_chat_input_key_press_event\" swapped=\"no\"/>	"\
		"              </object>                                                                                        "\
		"              <packing>                                                                                        "\
		"                <property name=\"expand\">True</property>                                                      "\
		"                <property name=\"fill\">True</property>                                                        "\
		"                <property name=\"position\">0</property>                                                       "\
		"              </packing>                                                                                       "\
		"            </child>                                                                                           "\
		"            <child>                                                                                            "\
		"              <object class=\"GtkButton\" id=\"send_btn\">                                                     "\
		"                <property name=\"label\" translatable=\"yes\">Send</property>                                  "\
		"                <property name=\"visible\">True</property>                                                     "\
		"                <property name=\"can_focus\">False</property>                                                  "\
		"                <property name=\"receives_default\">False</property>                                           "\
		"                <signal name=\"clicked\" handler=\"on_chat_send_btn_click_event\" swapped=\"no\"/>             "\
		"              </object>                                                                                        "\
		"              <packing>                                                                                        "\
		"                <property name=\"expand\">False</property>                                                     "\
		"                <property name=\"fill\">True</property>                                                        "\
		"                <property name=\"position\">1</property>                                                       "\
		"              </packing>                                                                                       "\
		"            </child>                                                                                           "\
		"          </object>                                                                                            "\
		"          <packing>                                                                                            "\
		"            <property name=\"expand\">False</property>                                                         "\
		"            <property name=\"fill\">True</property>                                                            "\
		"            <property name=\"position\">1</property>                                                           "\
		"          </packing>                                                                                           "\
		"        </child>                                                                                               "\
		"      </object>                                                                                                "\
		"    </child>                                                                                                   "\
		"  </object>                                                                                                    "\
		"</interface>																									"


#endif
