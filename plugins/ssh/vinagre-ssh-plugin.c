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

#include <vinagre/vinagre-cache-prefs.h>
#include <vinagre/vinagre-protocol.h>

#include "vinagre-ssh-plugin.h"
#include "vinagre-ssh-connection.h"
#include "vinagre-ssh-tab.h"

#ifdef VINAGRE_HAVE_AVAHI
#include <avahi-ui/avahi-ui.h>
#include <avahi-common/malloc.h>
#endif

void ssh_register_types (void);
static void vinagre_ssh_protocol_iface_init (VinagreProtocolInterface *iface);

G_DEFINE_TYPE_EXTENDED (VinagreSshPlugin,
			vinagre_ssh_plugin,
			VINAGRE_TYPE_STATIC_EXTENSION,
			0,
			G_IMPLEMENT_INTERFACE (VINAGRE_TYPE_PROTOCOL,
					       vinagre_ssh_protocol_iface_init))

static const gchar *
impl_get_protocol (VinagreProtocol *plugin)
{
  return "ssh";
}

static gchar **
impl_get_public_description (VinagreProtocol *plugin)
{
  gchar **result = g_new (gchar *, 3);

  result[0] = g_strdup (_("SSH"));
  /* Translators: This is a description of the SSH protocol. It appears at Connect dialog. */
  result[1] = g_strdup (_("Access Unix/Linux terminals"));
  result[2] = NULL;

  return result;
}

static const gchar *
impl_get_mdns_service (VinagreProtocol *plugin)
{
  return "_ssh._tcp";
}

static VinagreConnection *
impl_new_connection (VinagreProtocol *plugin)
{
  return vinagre_ssh_connection_new ();
}

static GtkWidget *
impl_new_tab (VinagreProtocol   *protocol,
	      VinagreConnection *conn,
	      VinagreWindow     *window)
{
  return vinagre_ssh_tab_new (conn, window);
}

static gint
impl_get_default_port (VinagreProtocol *plugin)
{
  return 22;
}

static GtkWidget *
impl_get_connect_widget (VinagreProtocol *plugin, VinagreConnection *conn)
{
  GtkWidget *box, *label, *u_box, *u_entry;
  gchar     *str;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);

  str = g_strdup_printf ("<b>%s</b>", _("SSH Options"));
  label = gtk_label_new (str);
  g_free (str);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  u_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

  label = gtk_label_new_with_mnemonic (_("_Username:"));
  gtk_box_pack_start (GTK_BOX (u_box), label, FALSE, FALSE, 0);

  u_entry = gtk_entry_new ();
  /* Translators: This is the tooltip for the username field in a SSH connection */
  gtk_widget_set_tooltip_text (u_entry, _("Optional. If blank, your username will be used. Also, it can be supplied in the Host field above, in the form username@hostname."));
  g_object_set_data (G_OBJECT (box), "username_entry", u_entry);
  gtk_container_add (GTK_CONTAINER (u_box), u_entry);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), u_entry);
  str = g_strdup (VINAGRE_IS_CONNECTION (conn) ?
		  vinagre_connection_get_username (conn) :
		  vinagre_cache_prefs_get_string  ("ssh-connection", "username", ""));
  gtk_entry_set_text (GTK_ENTRY (u_entry), str);
  gtk_entry_set_activates_default (GTK_ENTRY (u_entry), TRUE);
  g_free (str);

  gtk_widget_set_margin_left (u_box, 12);
  gtk_box_pack_start (GTK_BOX (box), u_box, TRUE, TRUE, 0);
  return box;
}

static void
ssh_parse_mdns_dialog (VinagreProtocol *plugin,
		       GtkWidget *connect_widget,
		       GtkWidget *dialog)
{
#ifdef VINAGRE_HAVE_AVAHI
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
vinagre_ssh_protocol_iface_init (VinagreProtocolInterface *iface)
{
  iface->get_protocol  = impl_get_protocol;
  iface->get_public_description  = impl_get_public_description;
  iface->new_connection = impl_new_connection;
  iface->get_mdns_service  = impl_get_mdns_service;
  iface->new_tab = impl_new_tab;
  iface->get_default_port = impl_get_default_port;
  iface->get_connect_widget = impl_get_connect_widget;
  iface->parse_mdns_dialog = ssh_parse_mdns_dialog;
}

static void
vinagre_ssh_plugin_class_finalize (VinagreSshPluginClass *klass)
{
}

static void
vinagre_ssh_plugin_class_init (VinagreSshPluginClass *klass)
{
}

static void
vinagre_ssh_plugin_init (VinagreSshPlugin *plugin)
{
}

__attribute__((constructor)) void
ssh_register_types (void)
{
  g_type_init ();
  volatile dontoptimiseaway = vinagre_ssh_plugin_get_type ();
}

/* vim: set ts=8: */
