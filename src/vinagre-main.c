/*
 * vinagre-main.c
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */


#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <config.h>
#include "vinagre-main.h"
#include "vinagre-prefs-manager.h"

int main (int argc, char **argv) {

  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  g_thread_init (NULL);

  gtk_init (&argc, &argv);
  g_set_application_name (_("Vinagre"));
  vinagre_prefs_manager_init ();

  main_window = vinagre_window_new ();

  gtk_widget_show (GTK_WIDGET(main_window));
  gtk_main ();

  vinagre_prefs_manager_shutdown ();

  return 0;
}
