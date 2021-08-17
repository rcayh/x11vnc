/*
   Copyright (C) 2002-2011 Karl J. Runge <runge@karlrunge.com> 
   All rights reserved.

This file is part of x11vnc.

x11vnc is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

x11vnc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with x11vnc; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
or see <http://www.gnu.org/licenses/>.

In addition, as a special exception, Karl J. Runge
gives permission to link the code of its release of x11vnc with the
OpenSSL project's "OpenSSL" library (or with modified versions of it
that use the same license as the "OpenSSL" library), and distribute
the linked executables.  You must obey the GNU General Public License
in all respects for all of the code used other than "OpenSSL".  If you
modify this file, you may extend this exception to your version of the
file, but you are not obligated to do so.  If you do not wish to do
so, delete this exception statement from your version.
*/

#ifndef _X11VNC_WIN_UTILS_H
#define _X11VNC_WIN_UTILS_H

/* -- win_utils.h -- */
#include "xinerama.h"
#include "winattr_t.h"

extern winattr_t *stack_list;
extern int stack_list_len;
extern int stack_list_num;

extern Window parent_window(Window win, char **name);
extern int valid_window(Window win, XWindowAttributes *attr_ret, int bequiet);
extern Bool xtranslate(Window src, Window dst, int src_x, int src_y, int *dst_x,
    int *dst_y, Window *child, int bequiet);
extern int get_window_size(Window win, int *w, int *h);
extern void snapshot_stack_list(int free_only, double allowed_age);
extern int get_boff(void);
extern int get_bwin(void);
extern void update_stack_list(void);
extern Window query_pointer(Window start);
extern unsigned int mask_state(void);
extern int pick_windowid(unsigned long *num);
extern Window descend_pointer(int depth, Window start, char *name_info, int len);
extern void id_cmd(char *cmd);

#endif /* _X11VNC_WIN_UTILS_H */
