/*
 * vinagre-utils.h
 * This file is part of vinagre
 *
 * Copyright (C) 2007 - Jonh Wendell <wendell@bani.com.br>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_UTILS_H__
#define __VINAGRE_UTILS_H__

#include <gtk/gtk.h>
#include <glib.h>

GtkWidget	*vinagre_utils_create_small_close_button (void);

void		vinagre_utils_show_error		(const gchar *message,
							 GtkWindow *parent);

void		vinagre_utils_show_many_errors		(const gchar *message,
							 GSList *items,
							 GtkWindow *parent);

void		vinagre_utils_toggle_widget_visible	(GtkWidget *widget);

const gchar	*vinagre_utils_get_glade_filename	(void);
const gchar	*vinagre_utils_get_ui_xml_filename	(void);

gchar		*vinagre_utils_escape_underscores	(const gchar *text,
							 gssize      length);

void		vinagre_utils_handle_debug		(void);
#endif  /* __VINAGRE_UTILS_H__  */
/* vim: ts=8 */
