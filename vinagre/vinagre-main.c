/*
 * vinagre-main.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008,2009 - Jonh Wendell <wendell@bani.com.br>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <locale.h>

#include "vinagre-connection.h"
#include "vinagre-commands.h"
#include "vinagre-bookmarks.h"
#include "vinagre-window.h"
#include "vinagre-app.h"
#include "vinagre-utils.h"
#include "vinagre-prefs.h"
#include "vinagre-bacon.h"
#include "vinagre-plugins-engine.h"
#include "vinagre-plugin-info.h"
#include "vinagre-plugin-info-priv.h"
#include "vinagre-debug.h"

#ifdef HAVE_TELEPATHY
#include "vinagre-tubes-manager.h"
#endif

#ifdef VINAGRE_ENABLE_AVAHI
#include "vinagre-mdns.h"
#endif

/* command line */
static gchar **files = NULL;
static gchar **remaining_args = NULL;
static GSList *servers = NULL;
static gboolean new_window = FALSE;
static gboolean fullscreen = FALSE;

static const GOptionEntry options [] =
{
  { "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &fullscreen,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Open vinagre in fullscreen mode"), NULL },

  { "new-window", 'n', 0, G_OPTION_ARG_NONE, &new_window,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Create a new toplevel window in an existing instance of vinagre"), NULL },

  { "file", 'F', 0, G_OPTION_ARG_FILENAME_ARRAY, &files,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Open a file recognized by vinagre"), N_("filename")},

  { 
    G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_STRING_ARRAY, &remaining_args,
  /* Translators: this is a command line option (run vinagre --help) */
    NULL, N_("[server:port]") },

  { NULL }
};

static void
vinagre_main_process_command_line (VinagreWindow *window)
{
  gint               i;
  VinagreConnection *conn;
  gchar             *error;
  GSList            *errors = NULL;

  if (files)
    {
      for (i = 0; files[i]; i++) 
	{
	  conn = vinagre_connection_new_from_file (files[i], &error, FALSE);
	  if (conn)
	    servers = g_slist_prepend (servers, conn);
	  else
	    {
	      errors = g_slist_prepend (errors,
					g_strdup_printf ("<i>%s</i>: %s",
							files[i],
							error ? error : _("Unknown error")));
	      if (error)
	        g_free (error);
	    }
	}
      g_strfreev (files);
    }

  if (remaining_args)
    {
      for (i = 0; remaining_args[i]; i++) 
	{
	  conn = vinagre_connection_new_from_string (remaining_args[i], &error, TRUE);
	  if (conn)
	    servers = g_slist_prepend (servers, conn);
	  else
	    errors = g_slist_prepend (errors,
				      g_strdup_printf ("<i>%s</i>: %s",
						       remaining_args[i],
						       error ? error : _("Unknown error")));

	  if (error)
	    g_free (error);
	}

      g_strfreev (remaining_args);
    }

  if (errors)
    {
      vinagre_utils_show_many_errors (ngettext ("The following error has occurred:",
						"The following errors have occurred:",
						g_slist_length (errors)),
				      errors,
				      window?GTK_WINDOW (window):NULL);
      g_slist_free (errors);
    }
}

int main (int argc, char **argv) {
  GOptionContext       *context;
  GError               *error = NULL;
  GSList               *l, *plugins;
  VinagreWindow        *window;
  VinagreApp           *app;
  VinagrePluginsEngine *engine;
#ifdef HAVE_TELEPATHY
  VinagreTubesManager *vinagre_tubes_manager;
#endif

  if (!g_thread_supported ())
    g_thread_init (NULL);
  g_type_init();

  /* Setup debugging */
  vinagre_debug_init ();
  vinagre_debug_message (DEBUG_APP, "Startup");

  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* Init plugins engine */
  vinagre_debug_message (DEBUG_APP, "Init plugins");
  engine = vinagre_plugins_engine_get_default ();
  plugins = (GSList *) vinagre_plugins_engine_get_plugin_list (engine);

  /* Setup command line options */
  context = g_option_context_new (_("- Remote Desktop Viewer"));
  g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));

  for (l = plugins; l; l = l->next)
    {
      GOptionGroup      *group;
      GSList            *groups, *l2;
      VinagrePluginInfo *info = VINAGRE_PLUGIN_INFO (l->data);

      if (!vinagre_plugin_info_is_active (info))
	continue;

      groups = vinagre_plugin_get_context_groups (info->plugin);
      for (l2 = groups; l2; l2 = l2->next)
	g_option_context_add_group (context, (GOptionGroup *)l2->data);
      g_slist_free (groups);
    }

  g_option_context_parse (context, &argc, &argv, &error);
  if (error)
    {
      g_print ("%s\n%s\n",
	       error->message,
	       _("Run 'vinagre --help' to see a full list of available command line options"));
      g_error_free (error);
      return 1;
    }

  g_set_application_name (_("Remote Desktop Viewer"));
  vinagre_main_process_command_line (NULL);

  vinagre_bacon_start (servers, new_window);

  app = vinagre_app_get_default ();
  window = vinagre_app_create_window (app, NULL);
  gtk_widget_show (GTK_WIDGET(window));

  vinagre_utils_handle_debug ();

  for (l = servers; l; l = l->next)
    {
      VinagreConnection *conn = l->data;

      vinagre_connection_set_fullscreen (conn, fullscreen);
      vinagre_cmd_direct_connect (conn, window);
      g_object_unref (conn);
    }
  g_slist_free (servers);

#ifdef HAVE_TELEPATHY
   vinagre_tubes_manager = vinagre_tubes_manager_new (window);
#endif

  gtk_main ();

#ifdef HAVE_TELEPATHY
  g_object_unref (vinagre_tubes_manager);
#endif
  g_object_unref (vinagre_bookmarks_get_default ());
  g_object_unref (vinagre_prefs_get_default ());
#ifdef VINAGRE_ENABLE_AVAHI
  g_object_unref (vinagre_mdns_get_default ());
#endif

  return 0;
}
/* vim: set ts=8: */
