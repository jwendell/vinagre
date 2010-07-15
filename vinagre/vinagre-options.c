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

#include "vinagre-options.h"
#include "vinagre-connection.h"
#include "vinagre-window.h"

const GOptionEntry all_options [] =
{
  { "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &optionstate.fullscreen,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Open vinagre in fullscreen mode"), NULL },

  { "new-window", 'n', 0, G_OPTION_ARG_NONE, &optionstate.new_window,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Create a new toplevel window in an existing instance of vinagre"), NULL },

  { "file", 'F', 0, G_OPTION_ARG_FILENAME_ARRAY, &optionstate.files,
  /* Translators: this is a command line option (run vinagre --help) */
    N_("Open a file recognized by vinagre"), N_("filename")},

  { 
    G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_STRING_ARRAY, &optionstate.uris,
  /* Translators: this is a command line option (run vinagre --help) */
    NULL, N_("[server:port]") },

  { NULL }
};

VinagreCmdLineOptions optionstate;

void
vinagre_options_register_actions (GtkApplication *app)
{
  GApplication *g_app = G_APPLICATION (app);
  g_application_add_action (g_app, "uris", "List of machines to connect to");
}

void
vinagre_options_process_command_line (GtkWindow *window, const VinagreCmdLineOptions *options)
{
  gint               i;
  VinagreConnection *conn;
  gchar             *error;
  GSList            *errors, *servers, *l;
  VinagreWindow     *v_window;
  GtkApplication    *app;

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

  app = GTK_APPLICATION (g_application_get_instance ());
  if (servers &&
      options->new_window)
    {
      v_window = vinagre_window_new ();
      gtk_widget_show (GTK_WIDGET (v_window));
      gtk_application_add_window (app, GTK_WINDOW (v_window));
    }
  else
    {
      v_window = VINAGRE_WINDOW (window);
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

void
vinagre_options_invoke_remote_instance (GtkApplication *app, const VinagreCmdLineOptions *options)
{
  GVariantBuilder builder;
  GVariant *data;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{sv}"));

  if (options->files)
    {
      g_variant_builder_add (&builder,
			     "{sv}",
			     "files",
			     g_variant_new_bytestring_array ((const gchar * const*)options->files, -1));
      g_strfreev (options->files);
    }

  if (options->uris)
    {
      g_variant_builder_add (&builder,
			     "{sv}",
			     "args",
			     g_variant_new_bytestring_array ((const gchar * const*)options->uris, -1));
      g_strfreev (options->uris);
    }

  g_variant_builder_add (&builder,
			 "{sv}",
			 "new_window",
			 g_variant_new_boolean (options->new_window));

  g_variant_builder_add (&builder,
			 "{sv}",
			 "fullscreen",
			 g_variant_new_boolean (options->fullscreen));


  data = g_variant_builder_end (&builder);
  g_application_invoke_action (G_APPLICATION (app), "uris", data);
  g_variant_unref (data);
}

void
vinagre_options_handle_action (GApplication *app,
			       gchar        *action,
			       GVariant     *data,
			       gpointer      user_data)
{
  GVariantIter iter;
  gchar *key;
  GVariant *value;
  const gchar **files, **uris;

  if (g_strcmp0 (action, "uris") != 0)
    {
      g_message ("Received an unknown action: %s", action);
      return;
    }

  files = uris = NULL;
  memset (&optionstate, 0, sizeof optionstate);

  g_variant_iter_init (&iter, data);
  while (g_variant_iter_loop (&iter, "{sv}", &key, &value))
    {
      if (g_strcmp0 (key, "files") == 0)
	{
	  optionstate.files = (gchar **) g_variant_dup_bytestring_array (value, NULL);
	}
      else if (g_strcmp0 (key, "args") == 0)
	{
	  optionstate.uris = (gchar **) g_variant_dup_bytestring_array (value, NULL);
	}
      else if (g_strcmp0 (key, "new_window") == 0)
	{
	  optionstate.new_window = g_variant_get_boolean (value);
	}
      else if (g_strcmp0 (key, "fullscreen") == 0)
	{
	  optionstate.fullscreen = g_variant_get_boolean (value);
	}
    }

  vinagre_options_process_command_line (gtk_application_get_window (GTK_APPLICATION (app)),
					&optionstate);
}
/* vim: set ts=8: */
