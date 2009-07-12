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

#include "vinagre-vnc-plugin.h"
#include "vinagre-vnc-connection.h"
#include "vinagre-vnc-tab.h"

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <vncdisplay.h>

#include <vinagre/vinagre-debug.h>
#include <vinagre/vinagre-utils.h>

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

static GOptionGroup *
impl_get_context_group (VinagrePlugin *plugin)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin Get Context Group");

  return vnc_display_get_option_group ();
}

static const gchar *
impl_get_protocol (VinagrePlugin *plugin)
{
  return "vnc";
}

static const gchar *
impl_get_mdns_service (VinagrePlugin *plugin)
{
  return "_rfb._tcp";
}

static VinagreConnection *
impl_new_connection (VinagrePlugin *plugin)
{
  return vinagre_vnc_connection_new ();
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
      if (!vinagre_connection_split_string (host, &protocol, &actual_host, &port, error_msg))
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
  plugin_class->get_context_group = impl_get_context_group;
  plugin_class->get_protocol  = impl_get_protocol;
  plugin_class->new_connection = impl_new_connection;
  plugin_class->new_connection_from_file = impl_new_connection_from_file;
  plugin_class->get_mdns_service  = impl_get_mdns_service;
  plugin_class->new_tab = impl_new_tab;
}
/* vim: set ts=8: */
