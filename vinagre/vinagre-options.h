/*
 * vinagre-options.h
 * This file is part of vinagre
 *
 * Copyright (C) 2010 - Jonh Wendell <wendell@bani.com.br>
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

#ifndef __VINAGRE_OPTIONS_H__
#define __VINAGRE_OPTIONS_H__

#include <gtk/gtk.h>
#include <glib/gi18n.h>

typedef struct {
  gchar **files;
  gchar **uris;
  gboolean new_window;
  gboolean fullscreen;
} VinagreCmdLineOptions;

extern const GOptionEntry all_options[];
extern VinagreCmdLineOptions optionstate;

void vinagre_options_register_actions       (GtkApplication *app);
void vinagre_options_process_command_line   (GtkWindow *window,
					     const VinagreCmdLineOptions *options);
void vinagre_options_invoke_remote_instance (GtkApplication *app,
					     const VinagreCmdLineOptions *options);
void vinagre_options_handle_action          (GApplication *app,
					     gchar        *action,
					     GVariant     *data,
					     gpointer      user_data);

#endif  /* __VINAGRE_OPTIONS_H__ */
/* vim: set ts=8: */
