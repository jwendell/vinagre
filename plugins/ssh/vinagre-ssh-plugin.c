/*
 * vinagre-ssh-plugin.c
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-ssh-plugin.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-ssh-plugin.c is distributed in the hope that it will be useful, but
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

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <vinagre/vinagre-debug.h>

#include "vinagre-ssh-plugin.h"
#include "vinagre-ssh-connection.h"
#include "vinagre-ssh-tab.h"

#define VINAGRE_SSH_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_SSH_PLUGIN, VinagreSshPluginPrivate))

VINAGRE_PLUGIN_REGISTER_TYPE(VinagreSshPlugin, vinagre_ssh_plugin)

static void
impl_activate (VinagrePlugin *plugin,
               VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreSshPlugin Activate");
}

static void
impl_deactivate  (VinagrePlugin *plugin,
                  VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreSshPlugin Deactivate");
}

static void
impl_update_ui (VinagrePlugin *plugin,
                VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreSshPlugin Update UI");
}

static const gchar *
impl_get_protocol (VinagrePlugin *plugin)
{
  return "ssh";
}

static gchar **
impl_get_public_description (VinagrePlugin *plugin)
{
  gchar **result = g_new (gchar *, 3);

  result[0] = g_strdup (_("SSH"));
  /* Translators: This is a description of the SSH protocol. It appears at Connect dialog. */
  result[1] = g_strdup (_("Access Unix/Linux terminals"));
  result[2] = NULL;

  return result;
}

static const gchar *
impl_get_mdns_service (VinagrePlugin *plugin)
{
  return "_ssh._tcp";
}

static VinagreConnection *
impl_new_connection (VinagrePlugin *plugin)
{
  return vinagre_ssh_connection_new ();
}

static GtkWidget *
impl_new_tab (VinagrePlugin     *plugin,
	      VinagreConnection *conn,
	      VinagreWindow     *window)
{
  return vinagre_ssh_tab_new (conn, window);
}

static gint
impl_get_default_port (VinagrePlugin *plugin)
{
  return 22;
}

static void
vinagre_ssh_plugin_init (VinagreSshPlugin *plugin)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreSshPlugin initializing");
}

static void
vinagre_ssh_plugin_finalize (GObject *object)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreSshPlugin finalizing");

  G_OBJECT_CLASS (vinagre_ssh_plugin_parent_class)->finalize (object);
}

static void
vinagre_ssh_plugin_class_init (VinagreSshPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  VinagrePluginClass *plugin_class = VINAGRE_PLUGIN_CLASS (klass);

  object_class->finalize   = vinagre_ssh_plugin_finalize;

  plugin_class->activate   = impl_activate;
  plugin_class->deactivate = impl_deactivate;
  plugin_class->update_ui  = impl_update_ui;
  plugin_class->get_protocol  = impl_get_protocol;
  plugin_class->get_public_description  = impl_get_public_description;
  plugin_class->new_connection = impl_new_connection;
  plugin_class->get_mdns_service  = impl_get_mdns_service;
  plugin_class->new_tab = impl_new_tab;
  plugin_class->get_default_port = impl_get_default_port;
}
/* vim: set ts=8: */
