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
#include <glib/goption.h>
#include <stdlib.h>

#include <config.h>
#include "vinagre-main.h"
#include "vinagre-prefs-manager.h"
#include "vinagre-connection.h"
#include "vinagre-commands.h"
#include "vinagre-favorites.h"

/* command line */
static gchar **remaining_args = NULL;
static GSList *servers = NULL;

static const GOptionEntry options [] =
{
	{ G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &remaining_args,
	  NULL, N_("[server:port]") },

	{ NULL }
};

static void
vinagre_main_process_command_line (void)
{
  gint i, port;
  gchar *host;
  gchar **server;
  VinagreConnection *conn;

  if (remaining_args)
    {
      for (i = 0; remaining_args[i]; i++) 
	{
	  server = g_strsplit (remaining_args[i], ":", 2);
	  host = server[0];
	  port = server[1] ? atoi (server[1]) : 5900;

	  conn = vinagre_favorites_exists (host, port);
	  if (!conn)
	    {
	      conn = vinagre_connection_new ();
	      vinagre_connection_set_host (conn, host);
	      vinagre_connection_set_port (conn, port);
	    }

	  servers = g_slist_append (servers, conn);
	  g_strfreev (server);
	}
    }
}

int main (int argc, char **argv) {
  GOptionContext *context;
  GError *error = NULL;
  GSList *l, *next;

  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* Setup command line options */
  context = g_option_context_new (_("- VNC Client for GNOME"));
  g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_parse (context, &argc, &argv, &error);

  if (!g_thread_supported ())
    g_thread_init (NULL);

  g_set_application_name (_("Vinagre"));
  vinagre_prefs_manager_init ();

  main_window = vinagre_window_new ();
  gtk_widget_show (GTK_WIDGET(main_window));

  vinagre_main_process_command_line ();
  for (l = servers; l; l = next)
    {
      VinagreConnection *conn = l->data;
      
      next = l->next;
      vinagre_cmd_direct_connect (conn, main_window);
    }
  g_slist_free (servers);

  gtk_main ();

  vinagre_prefs_manager_shutdown ();

  return 0;
}
