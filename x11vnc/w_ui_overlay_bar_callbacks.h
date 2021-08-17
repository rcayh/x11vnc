

#ifndef W_UI_OVERLAY_BAR_CALLBACKS_H_
#define W_UI_OVERLAY_BAR_CALLBACKS_H_

#include <gtk/gtk.h>

/* full opaque alpha */
#define OPAQUE_ALPHA "FF"

/* Semi opaque (and then semi transparent) alpha;
 * this is used to make the highlighter effect.
 */
#define SEMI_OPAQUE_ALPHA "88"

/* The time-out after that the tool try to up-rise the window;
 * this is done to prevent the window lowering in the case that
 * the window manager does not support the stay above directive.
 */
#define  BAR_TO_TOP_TIMEOUT 1000

#endif

