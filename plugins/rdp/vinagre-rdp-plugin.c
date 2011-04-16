/*
 * vinagre-rdp-plugin.c
 * This file is part of vinagre
 *
 * Copyright (C) 2010 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-rdp-plugin.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-rdp-plugin.c is distributed in the hope that it will be useful, but
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

#include "vinagre-rdp-plugin.h"
#include "vinagre-rdp-connection.h"
#include "vinagre-rdp-tab.h"

static void vinagre_rdp_protocol_iface_init (VinagreProtocolInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (VinagreRdpPlugin,
				vinagre_rdp_plugin,
				PEAS_TYPE_EXTENSION_BASE,
				0,
				G_IMPLEMENT_INTERFACE_DYNAMIC (VINAGRE_TYPE_PROTOCOL,
							       vinagre_rdp_protocol_iface_init))

static const gchar *
impl_get_protocol (VinagreProtocol *plugin)
{
  return "rdp";
}

static gchar **
impl_get_public_description (VinagreProtocol *plugin)
{
  gchar **result = g_new (gchar *, 3);

  result[0] = g_strdup (_("RDP"));
  /* Translators: This is a description of the RDP protocol. It appears in the Connect dialog. */
  result[1] = g_strdup (_("Access MS Windows remote desktops"));
  result[2] = NULL;

  return result;
}

static VinagreConnection *
impl_new_connection (VinagreProtocol *plugin)
{
  return vinagre_rdp_connection_new ();
}

static GtkWidget *
impl_new_tab (VinagreProtocol   *plugin,
	      VinagreConnection *conn,
	      VinagreWindow     *window)
{
  return vinagre_rdp_tab_new (conn, window);
}

static gint
impl_get_default_port (VinagreProtocol *plugin)
{
  return 3389;
}

static void
vinagre_rdp_plugin_init (VinagreRdpPlugin *plugin)
{
}

static GtkWidget *
impl_get_connect_widget (VinagreProtocol *plugin, VinagreConnection *conn)
{
  GtkWidget *box, *label, *u_box, *u_entry;
  gchar     *str;

  box = gtk_vbox_new (FALSE, 0);

  str = g_strdup_printf ("<b>%s</b>", _("RDP Options"));
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
  /* Translators: This is the tooltip for the username field in a RDP connection */
  gtk_widget_set_tooltip_text (u_entry, _("Optional. If blank, your username will be used. Also, it can be supplied in the Host field above, in the form username@hostname."));
  g_object_set_data (G_OBJECT (box), "username_entry", u_entry);
  gtk_box_pack_start (GTK_BOX (u_box), u_entry, TRUE, TRUE, 5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), u_entry);
  str = g_strdup (VINAGRE_IS_CONNECTION (conn) ?
		  vinagre_connection_get_username (conn) :
		  vinagre_cache_prefs_get_string  ("rdp-connection", "username", ""));
  gtk_entry_set_text (GTK_ENTRY (u_entry), str);
  gtk_entry_set_activates_default (GTK_ENTRY (u_entry), TRUE);
  g_free (str);

  gtk_box_pack_start (GTK_BOX (box), u_box, TRUE, TRUE, 0);
  return box;
}

static void
vinagre_rdp_protocol_iface_init (VinagreProtocolInterface *iface)
{
  iface->get_protocol  = impl_get_protocol;
  iface->get_public_description  = impl_get_public_description;
  iface->new_connection = impl_new_connection;
  iface->new_tab = impl_new_tab;
  iface->get_default_port = impl_get_default_port;
  iface->get_connect_widget = impl_get_connect_widget;
}

static void
vinagre_rdp_plugin_class_finalize (VinagreRdpPluginClass *klass)
{
}

static void
vinagre_rdp_plugin_class_init (VinagreRdpPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  vinagre_rdp_plugin_register_type (G_TYPE_MODULE (module));
  peas_object_module_register_extension_type (module,
					      VINAGRE_TYPE_PROTOCOL,
					      VINAGRE_TYPE_RDP_PLUGIN);
}

/* vim: set ts=8: */
