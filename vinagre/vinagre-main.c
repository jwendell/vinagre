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

#ifdef ENABLE_INTROSPECTION
#include <girepository.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <locale.h>

#include "vinagre-window.h"
#include "vinagre-utils.h"
#include "vinagre-prefs.h"
#include "vinagre-cache-prefs.h"
#include "vinagre-debug.h"
#include "vinagre-ssh.h"
#include "vinagre-options.h"
#include "vinagre-plugins-engine.h"

#ifdef HAVE_TELEPATHY
#include "vinagre-tubes-manager.h"
#endif

#ifdef VINAGRE_ENABLE_AVAHI
#include "vinagre-mdns.h"
#endif

int main (int argc, char **argv) {
  GOptionContext       *context;
  GError               *error = NULL;
  GtkWindow            *window;
  GtkApplication       *app;
  GHashTable           *extensions;
  GHashTableIter        iter;
  VinagreProtocol      *extension;
#ifdef HAVE_TELEPATHY
  VinagreTubesManager *vinagre_tubes_manager;
#endif

  g_type_init();
  g_set_application_name (_("Remote Desktop Viewer"));

  /* Setup debugging */
  vinagre_debug_init ();
  vinagre_debug_message (DEBUG_APP, "Startup");

  /* i18n */
  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* Init plugins engine */
  vinagre_debug_message (DEBUG_APP, "Init plugins");
  extensions = vinagre_plugins_engine_get_plugins_by_protocol (vinagre_plugins_engine_get_default ());

  /* Setup command line options */
  context = g_option_context_new (_("- Remote Desktop Viewer"));
  g_option_context_add_main_entries (context, all_options, GETTEXT_PACKAGE);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));

#ifdef ENABLE_INTROSPECTION
  g_option_context_add_group (context, g_irepository_get_option_group ());
#endif

  g_hash_table_iter_init (&iter, extensions);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *)&extension))
    {
      GSList *groups, *l;

      groups = vinagre_protocol_get_context_groups (extension);
      for (l = groups; l; l = l->next)
	g_option_context_add_group (context, (GOptionGroup *)l->data);
      g_slist_free (groups);
    }

  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);
  if (error)
    {
      g_print ("%s\n%s\n",
	       error->message,
	       _("Run 'vinagre --help' to see a full list of available command line options"));
      g_error_free (error);
      return 1;
    }

  g_clear_error (&error);
  app = g_initable_new (GTK_TYPE_APPLICATION,
			NULL,
			&error,
			"application-id", "org.gnome.Vinagre",
			"argv", g_variant_new_bytestring_array ((const gchar * const*)argv, argc),
			"default-quit", FALSE,
			NULL);
  if (!app)
    g_error ("%s", error->message);

  if (g_application_is_remote (G_APPLICATION (app)))
    {
      vinagre_options_invoke_remote_instance (app, &optionstate);
      return 0;
    }

  vinagre_options_register_actions (app);
  vinagre_cache_prefs_init ();

  window = GTK_WINDOW (vinagre_window_new ());
  gtk_application_add_window (app, window);
  gtk_widget_show (GTK_WIDGET (window));

  vinagre_utils_handle_debug ();
  optionstate.new_window = FALSE;
  vinagre_options_process_command_line (window, &optionstate);

#ifdef HAVE_TELEPATHY
   vinagre_tubes_manager = vinagre_tubes_manager_new (window);
#endif

  /* fake call, just to ensure this symbol will be present at vinagre.so */
  vinagre_ssh_connect (NULL, NULL, -1, NULL, NULL, NULL, NULL, NULL);

  g_signal_connect (app,
		    "action-with-data",
		    G_CALLBACK (vinagre_options_handle_action),
		    NULL);
  gtk_application_run (app);

#ifdef HAVE_TELEPATHY
  g_object_unref (vinagre_tubes_manager);
#endif
  g_object_unref (vinagre_bookmarks_get_default ());
  g_object_unref (vinagre_prefs_get_default ());
  vinagre_cache_prefs_finalize ();
  g_object_unref (app);
#ifdef VINAGRE_ENABLE_AVAHI
  g_object_unref (vinagre_mdns_get_default ());
#endif


  return 0;
}
/* vim: set ts=8: */
