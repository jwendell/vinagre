/*
 * vinagre-options.c
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

#include <config.h>
#include "vinagre-options.h"
#include "vinagre-connection.h"
#include "vinagre-window.h"
#include "vinagre-commands.h"
#include "vinagre-options.h"
#include "vinagre-vala.h"

const GOptionEntry all_options [] =
{
  { "geometry", 0, 0, G_OPTION_ARG_STRING, &optionstate.geometry,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Specify geometry of the main Vinagre window"), NULL },

  { "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &optionstate.fullscreen,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Open Vinagre in fullscreen mode"), NULL },

  { "new-window", 'n', 0, G_OPTION_ARG_NONE, &optionstate.new_window,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Create a new toplevel window in an existing instance of Vinagre"), NULL },

  { "file", 'F', 0, G_OPTION_ARG_FILENAME_ARRAY, &optionstate.files,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Open a file recognized by Vinagre"), N_("filename")},

  { "help", '?', 0, G_OPTION_ARG_NONE, &optionstate.help,
    N_("Show help"), NULL},

  {
    G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_STRING_ARRAY, &optionstate.uris,
  /* Translators: this is a command line option (run vinagre --help) */
    NULL, N_("[server:port]") },

  { NULL }
};

VinagreCmdLineOptions optionstate;

void
vinagre_options_process_command_line (GtkApplication *app,
				      GtkWindow      *window,
				      const VinagreCmdLineOptions *options)
{
  gint               i;
  VinagreConnection *conn;
  gchar             *error;
  GSList            *errors, *servers, *l;
  VinagreWindow     *v_window;

  errors = servers = NULL;

  if (options->files)
    {
      for (i = 0; options->files[i]; i++) 
	{
	  conn = vinagre_connection_new_from_file (options->files[i], &error, FALSE);
	  if (conn)
	    servers = g_slist_prepend (servers, conn);
	  else
	    {
	      errors = g_slist_prepend (errors,
					g_strdup_printf ("<i>%s</i>: %s",
							options->files[i],
							error ? error : _("Unknown error")));
	      g_free (error);
	    }
	}
      g_strfreev (options->files);
    }

  if (options->uris)
    {
      for (i = 0; options->uris[i]; i++) 
	{
	  conn = vinagre_connection_new_from_string (options->uris[i], &error, TRUE);
	  if (conn)
	    servers = g_slist_prepend (servers, conn);
	  else
	    errors = g_slist_prepend (errors,
				      g_strdup_printf ("<i>%s</i>: %s",
						       options->uris[i],
						       error ? error : _("Unknown error")));
	  g_free (error);
	}

      g_strfreev (options->uris);
    }

  if (servers &&
      options->new_window)
    {
      v_window = vinagre_window_new ();
      gtk_widget_show_all (GTK_WIDGET (v_window));
      gtk_window_set_application (GTK_WINDOW (v_window), app);
    }
  else
    {
      v_window = VINAGRE_WINDOW (window);
    }

  if (options->geometry)
    {
      if (!gtk_window_parse_geometry (window, options->geometry))
        {
	  errors = g_slist_prepend (errors,
              g_strdup_printf (_("Invalid argument %s for --geometry"),
                               options->geometry));
        }
      gtk_window_unmaximize (window);
    }

  for (l = servers; l; l = l->next)
    {
      VinagreConnection *conn = l->data;

      vinagre_connection_set_fullscreen (conn, options->fullscreen);
      vinagre_cmd_direct_connect (conn, v_window);
      g_object_unref (conn);
    }
  g_slist_free (servers);

  if (errors)
    {
      vinagre_utils_show_many_errors (ngettext ("The following error has occurred:",
						"The following errors have occurred:",
						g_slist_length (errors)),
				      errors,
				      window);
      g_slist_free (errors);
    }

  gtk_window_present (GTK_WINDOW (v_window));
}

/* vim: set ts=8: */
