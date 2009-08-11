/*
 * vinagre-vnc-plugin.c
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-vnc-plugin.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-vnc-plugin.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <vncdisplay.h>

#include <vinagre/vinagre-debug.h>
#include <vinagre/vinagre-utils.h>

#include "vinagre-vnc-plugin.h"
#include "vinagre-vnc-connection.h"
#include "vinagre-vnc-tab.h"

#define VINAGRE_VNC_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_VNC_PLUGIN, VinagreVncPluginPrivate))

VINAGRE_PLUGIN_REGISTER_TYPE(VinagreVncPlugin, vinagre_vnc_plugin)

static void
impl_activate (VinagrePlugin *plugin,
               VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin Activate");
}

static void
impl_deactivate  (VinagrePlugin *plugin,
                  VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin Deactivate");
}

static void
impl_update_ui (VinagrePlugin *plugin,
                VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin Update UI");
}

static const GOptionEntry vinagre_vnc_args[] =
{
  { "vnc-scale", 0, 0, G_OPTION_ARG_NONE, &scaling_command_line,
  /* Translators: this is a command line option (run vinagre --help) */
  N_("Enable scaled mode"), 0 },
  { NULL }
};

static GSList *
impl_get_context_groups (VinagrePlugin *plugin)
{
  GOptionGroup *group;
  GSList       *groups = NULL;

  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin Get Context Group");

  scaling_command_line = FALSE;
  group = g_option_group_new ("vnc",
			      /* Translators: this is a command line option (run vinagre --help) */
			      _("VNC Options:"),
			      /* Translators: this is a command line option (run vinagre --help) */
			      _("Show VNC Options"),
			      NULL,
			      NULL);
  g_option_group_add_entries (group, vinagre_vnc_args);

  groups = g_slist_append (groups, group);
  groups = g_slist_append (groups, vnc_display_get_option_group ());

  return groups;
}

static const gchar *
impl_get_protocol (VinagrePlugin *plugin)
{
  return "vnc";
}

static gchar **
impl_get_public_description (VinagrePlugin *plugin)
{
  gchar **result = g_new (gchar *, 3);

  result[0] = g_strdup (_("VNC"));
  result[1] = g_strdup (_("Access Unix/Linux, Windows and other machines."));
  result[2] = NULL;

  return result;
}

static const gchar *
impl_get_mdns_service (VinagrePlugin *plugin)
{
  return "_rfb._tcp";
}

static VinagreConnection *
impl_new_connection (VinagrePlugin *plugin)
{
  VinagreConnection *conn;

  conn = vinagre_vnc_connection_new ();
  vinagre_vnc_connection_set_scaling (VINAGRE_VNC_CONNECTION (conn),
				      scaling_command_line);

  return conn;
}

static VinagreConnection *
impl_new_connection_from_file (VinagrePlugin *plugin,
			       const gchar   *data,
			       gboolean       use_bookmarks,
			       gchar        **error_msg)
{
  GKeyFile          *file;
  GError            *error;
  gboolean           loaded;
  gchar             *host, *actual_host, *protocol;
  gint               port;
  VinagreConnection *conn;

  *error_msg = NULL;
  conn = NULL;
  host = NULL;
  protocol = NULL;
  error = NULL;

  file = g_key_file_new ();
  loaded = g_key_file_load_from_data (file,
				      data,
				      -1,
				      0,
				      &error);
  if (!loaded)
    {
      if (error)
	{
	  *error_msg = g_strdup (error->message);
	  g_error_free (error);
	}
      else
	*error_msg = g_strdup (_("Could not parse the file."));

      goto the_end;
    }

  if (!g_key_file_has_group (file, "connection"))
    {
      *error_msg = g_strdup (_("The file is not a VNC one: Missing the group \"connection\"."));
      goto the_end;
    }

  if (!g_key_file_has_key (file, "connection", "host", NULL))
    {
      *error_msg = g_strdup (_("The file is not a VNC one: Missing the key \"host\"."));
      goto the_end;
    }

  host = g_key_file_get_string (file, "connection", "host", NULL);
  port = g_key_file_get_integer (file, "connection", "port", NULL);
  if (!port)
    {
      if (!vinagre_connection_split_string (host, "vnc", &protocol, &actual_host, &port, error_msg))
	goto the_end;

      g_free (host);
      host = actual_host;
    }

  if (use_bookmarks)
    conn = vinagre_bookmarks_exists (vinagre_bookmarks_get_default (), "vnc", host, port);
  if (!conn)
    {
      gchar *s_value;
      gint shared;

      conn = vinagre_vnc_connection_new ();
      vinagre_connection_set_host (conn, host);
      vinagre_connection_set_port (conn, port);

      s_value = g_key_file_get_string  (file, "connection", "username", NULL);
      vinagre_connection_set_username (conn, s_value);
      g_free (s_value);

      s_value = g_key_file_get_string  (file, "connection", "password", NULL);
      vinagre_connection_set_password (conn, s_value);
      g_free (s_value);

      shared = g_key_file_get_integer (file, "options", "shared", NULL);
      if (shared == 0 || shared == 1)
	vinagre_vnc_connection_set_shared (VINAGRE_VNC_CONNECTION (conn), shared);
      else
	g_message (_("Bad value for 'shared' flag: %d. It is supposed to be 0 or 1. Ignoring it."), shared);
    }

the_end:

  g_free (host);
  g_free (protocol);
  g_key_file_free (file);
  return conn;

}

static GtkWidget *
impl_new_tab (VinagrePlugin *plugin,
	      VinagreConnection *conn,
	      VinagreWindow     *window)
{
  return vinagre_vnc_tab_new (conn, window);
}

static GtkWidget *
impl_get_connect_widget (VinagrePlugin *plugin, VinagreConnection *conn)
{
  GtkWidget *box, *check, *label;
  GtkTable  *table;
  gchar     *str;

  box = gtk_vbox_new (TRUE, 0);

  str = g_strdup_printf ("<b>%s</b>", _("VNC Options"));
  label = gtk_label_new (str);
  g_free (str);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  table = GTK_TABLE (gtk_table_new (2, 2, FALSE));
  label = gtk_label_new ("  ");
  gtk_table_attach (table, label, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);

  check = gtk_check_button_new_with_mnemonic (_("_View only"));
  g_object_set_data (G_OBJECT (box), "view_only", check);
  gtk_table_attach_defaults (table, check, 1, 2, 0, 1);
  if (VINAGRE_IS_VNC_CONNECTION (conn))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check),
				  vinagre_vnc_connection_get_view_only (VINAGRE_VNC_CONNECTION (conn)));

  check = gtk_check_button_new_with_mnemonic (_("_Scaling"));
  g_object_set_data (G_OBJECT (box), "scaling", check);
  gtk_table_attach_defaults (table, check, 1, 2, 1, 2);
  if (VINAGRE_IS_VNC_CONNECTION (conn))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check),
				  vinagre_vnc_connection_get_scaling (VINAGRE_VNC_CONNECTION (conn)));

  gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (table), TRUE, TRUE, 0);
  return box;
}

static gint
impl_get_default_port (VinagrePlugin *plugin)
{
  return 5900;
}

static GtkFileFilter *
impl_get_file_filter (VinagrePlugin *plugin)
{
  GtkFileFilter *filter;

  filter = gtk_file_filter_new ();
  /* Translators: this is a pattern to open *.vnc files in a open dialog. */
  gtk_file_filter_set_name (filter, _("VNC Files"));
  gtk_file_filter_add_pattern (filter, "*.vnc");

  return filter;
}

static void
vinagre_vnc_plugin_init (VinagreVncPlugin *plugin)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin initializing");
}

static void
vinagre_vnc_plugin_finalize (GObject *object)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin finalizing");

  G_OBJECT_CLASS (vinagre_vnc_plugin_parent_class)->finalize (object);
}

static void
vinagre_vnc_plugin_class_init (VinagreVncPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  VinagrePluginClass *plugin_class = VINAGRE_PLUGIN_CLASS (klass);

  object_class->finalize   = vinagre_vnc_plugin_finalize;

  plugin_class->activate   = impl_activate;
  plugin_class->deactivate = impl_deactivate;
  plugin_class->update_ui  = impl_update_ui;
  plugin_class->get_context_groups = impl_get_context_groups;
  plugin_class->get_protocol  = impl_get_protocol;
  plugin_class->get_public_description  = impl_get_public_description;
  plugin_class->new_connection = impl_new_connection;
  plugin_class->new_connection_from_file = impl_new_connection_from_file;
  plugin_class->get_mdns_service  = impl_get_mdns_service;
  plugin_class->new_tab = impl_new_tab;
  plugin_class->get_connect_widget = impl_get_connect_widget;
  plugin_class->get_default_port = impl_get_default_port;
  plugin_class->get_file_filter = impl_get_file_filter;
}
/* vim: set ts=8: */
