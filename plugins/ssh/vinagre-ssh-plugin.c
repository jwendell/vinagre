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
#include <vinagre/vinagre-cache-prefs.h>

#include "vinagre-ssh-plugin.h"
#include "vinagre-ssh-connection.h"
#include "vinagre-ssh-tab.h"

#ifdef VINAGRE_ENABLE_AVAHI
#include <avahi-ui/avahi-ui.h>
#include <avahi-common/malloc.h>
#endif

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

static GtkWidget *
impl_get_connect_widget (VinagrePlugin *plugin, VinagreConnection *conn)
{
  GtkWidget *box, *label, *u_box, *u_entry;
  gchar     *str;

  box = gtk_vbox_new (FALSE, 0);

  str = g_strdup_printf ("<b>%s</b>", _("SSH Options"));
  label = gtk_label_new (str);
  g_free (str);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 0, 6);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  u_box = gtk_hbox_new (FALSE, 4);
  label = gtk_label_new ("  ");
  gtk_box_pack_start (GTK_BOX (u_box), label, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Username:"));
  gtk_box_pack_start (GTK_BOX (u_box), label, FALSE, FALSE, 0);

  u_entry = gtk_entry_new ();
  /* Translators: This is the tooltip for the username field in a SSH connection */
  gtk_widget_set_tooltip_text (u_entry, _("Optional. If blank, your username will be used. Also, it can be supplied in the Machine field above, in the form username@hostname."));
  g_object_set_data (G_OBJECT (box), "username_entry", u_entry);
  gtk_box_pack_start (GTK_BOX (u_box), u_entry, TRUE, TRUE, 5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), u_entry);
  str = g_strdup (VINAGRE_IS_CONNECTION (conn) ?
		  vinagre_connection_get_username (conn) :
		  vinagre_cache_prefs_get_string  ("ssh-connection", "username", ""));
  gtk_entry_set_text (GTK_ENTRY (u_entry), str);
  gtk_entry_set_activates_default (GTK_ENTRY (u_entry), TRUE);
  g_free (str);

  gtk_box_pack_start (GTK_BOX (box), u_box, TRUE, TRUE, 0);
  return box;
}

static void
ssh_parse_mdns_dialog (VinagrePlugin *plugin,
		       GtkWidget *connect_widget,
		       GtkWidget *dialog)
{
#ifdef VINAGRE_ENABLE_AVAHI
  const AvahiStringList *txt;
  gchar *u = NULL;

  for (txt = aui_service_dialog_get_txt_data (AUI_SERVICE_DIALOG (dialog)); txt; txt = txt->next)
    {
      char *key, *value;

      if (avahi_string_list_get_pair ((AvahiStringList*) txt, &key, &value, NULL) < 0)
	break;

      if (strcmp(key, "u") == 0)
	u = g_strdup(value);

      avahi_free (key);
      avahi_free (value);
    }

  if (u)
    {
      GtkEntry *u_entry = g_object_get_data (G_OBJECT (connect_widget), "username_entry");

      if (u_entry)
        gtk_entry_set_text (u_entry, u);
      else
	g_warning ("Wrong widget passed to ssh_parse_mdns_dialog()");

      g_free (u);
    }
#endif
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
  plugin_class->get_connect_widget = impl_get_connect_widget;
  plugin_class->parse_mdns_dialog = ssh_parse_mdns_dialog;
}
/* vim: set ts=8: */
