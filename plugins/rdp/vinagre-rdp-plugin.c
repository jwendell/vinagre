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

#define DEFAULT_WIDTH   800
#define DEFAULT_HEIGHT  600
#define MIN_SIZE          1
#define MAX_SIZE       8192

void rdp_register_types (void);
static void vinagre_rdp_protocol_iface_init (VinagreProtocolInterface *iface);

G_DEFINE_TYPE_EXTENDED (VinagreRdpPlugin,
			vinagre_rdp_plugin,
			VINAGRE_TYPE_STATIC_EXTENSION,
			0,
			G_IMPLEMENT_INTERFACE (VINAGRE_TYPE_PROTOCOL,
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
  GtkWidget *grid, *label, *u_entry, *spin_button;
  gchar     *str;

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 6);

  str = g_strdup_printf ("<b>%s</b>", _("RDP Options"));
  label = gtk_label_new (str);
  g_free (str);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

  label = gtk_label_new_with_mnemonic (_("_Username:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);
  gtk_widget_set_margin_left (label, 12);

  u_entry = gtk_entry_new ();
  /* Translators: This is the tooltip for the username field in a RDP connection */
  gtk_widget_set_tooltip_text (u_entry, _("Optional. If blank, your username will be used. Also, it can be supplied in the Host field above, in the form username@hostname."));
  g_object_set_data (G_OBJECT (grid), "username_entry", u_entry);
  gtk_grid_attach (GTK_GRID (grid), u_entry, 1, 1, 1, 1);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), u_entry);
  str = g_strdup (VINAGRE_IS_CONNECTION (conn) ?
		  vinagre_connection_get_username (conn) :
		  vinagre_cache_prefs_get_string  ("rdp-connection", "username", ""));
  gtk_entry_set_text (GTK_ENTRY (u_entry), str);
  gtk_entry_set_activates_default (GTK_ENTRY (u_entry), TRUE);
  g_free (str);


  /* Host width */
  label = gtk_label_new_with_mnemonic (_("_Width:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 2, 1, 1);
  gtk_widget_set_margin_left (label, 12);

  spin_button = gtk_spin_button_new_with_range (MIN_SIZE, MAX_SIZE, 1);
  /* Translators: This is the tooltip for the width field in a RDP connection */
  gtk_widget_set_tooltip_text (spin_button, _("Set width of the remote desktop"));
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_button), DEFAULT_WIDTH);
  g_object_set_data (G_OBJECT (grid), "width_spin_button", spin_button);
  gtk_grid_attach (GTK_GRID (grid), spin_button, 1, 2, 1, 1);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), spin_button);
  gtk_entry_set_activates_default (GTK_ENTRY (spin_button), TRUE);


  /* Host height */
  label = gtk_label_new_with_mnemonic (_("_Height:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 3, 1, 1);
  gtk_widget_set_margin_left (label, 12);

  spin_button = gtk_spin_button_new_with_range (MIN_SIZE, MAX_SIZE, 1);
  /* Translators: This is the tooltip for the height field in a RDP connection */
  gtk_widget_set_tooltip_text (spin_button, _("Set height of the remote desktop"));
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_button), DEFAULT_HEIGHT);
  g_object_set_data (G_OBJECT (grid), "height_spin_button", spin_button);
  gtk_grid_attach (GTK_GRID (grid), spin_button, 1, 3, 1, 1);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), spin_button);
  gtk_entry_set_activates_default (GTK_ENTRY (spin_button), TRUE);


  return grid;
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

__attribute__((constructor)) void
rdp_register_types (void)
{
  g_type_init ();
  volatile dontoptimiseaway = vinagre_rdp_plugin_get_type ();
}

/* vim: set ts=8: */
