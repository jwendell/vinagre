/*
 * vinagre-connect.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007 - Jonh Wendell <wendell@bani.com.br>
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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib.h>
#include <glade/glade.h>

#include "vinagre-connect.h"
#include "vinagre-utils.h"
#include "vinagre-bookmarks.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef VINAGRE_HAVE_AVAHI
#include <avahi-ui/avahi-ui.h>
#endif

typedef struct {
  GladeXML  *xml;
  GtkWidget *dialog;
  GtkWidget *host_entry;
  GtkWidget *port_entry;
  GtkWidget *find_button;
} VinagreConnectDialog;

#ifdef VINAGRE_HAVE_AVAHI
static void
vinagre_connect_find_button_cb (GtkButton            *button,
				VinagreConnectDialog *dialog)
{
  GtkWidget *d;

  d = aui_service_dialog_new (_("Choose a VNC Server"),
				GTK_WINDOW(dialog->dialog),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				NULL);
  gtk_window_set_transient_for (GTK_WINDOW(d), GTK_WINDOW(dialog->dialog));
  aui_service_dialog_set_resolve_service (AUI_SERVICE_DIALOG(d), TRUE);
  aui_service_dialog_set_resolve_host_name (AUI_SERVICE_DIALOG(d), TRUE);
  aui_service_dialog_set_browse_service_types (AUI_SERVICE_DIALOG(d),
					       "_rfb._tcp",
					       NULL);

  if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT)
    {
      gtk_entry_set_text (GTK_ENTRY (dialog->host_entry),
			  aui_service_dialog_get_host_name(AUI_SERVICE_DIALOG(d)));

      gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->port_entry),
				 aui_service_dialog_get_port(AUI_SERVICE_DIALOG(d)));
    }

  gtk_widget_destroy (d);
}
#endif

VinagreConnection *vinagre_connect (VinagreWindow *window)
{
  VinagreConnection    *conn = NULL;
  gint                  result;
  const gchar          *host;
  int                   port;
  VinagreConnectDialog  dialog;

  dialog.xml = glade_xml_new (vinagre_utils_get_glade_filename (), NULL, NULL);
  dialog.dialog = glade_xml_get_widget (dialog.xml, "connect_dialog");
  gtk_window_set_transient_for (GTK_WINDOW (dialog.dialog), GTK_WINDOW (window));

  dialog.host_entry  = glade_xml_get_widget (dialog.xml, "host_entry");
  dialog.port_entry  = glade_xml_get_widget (dialog.xml, "port_entry");
  dialog.find_button = glade_xml_get_widget (dialog.xml, "find_button");

#ifdef VINAGRE_HAVE_AVAHI
  gtk_widget_show (dialog.find_button);
  g_signal_connect (dialog.find_button,
		    "clicked",
		    G_CALLBACK (vinagre_connect_find_button_cb),
		    &dialog);
#endif

  gtk_widget_show_all (dialog.dialog);
  result = gtk_dialog_run (GTK_DIALOG (dialog.dialog));

  if (result == GTK_RESPONSE_OK)
    {
      host = gtk_entry_get_text (GTK_ENTRY(dialog.host_entry));
      port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog.port_entry));

      gtk_widget_hide (GTK_WIDGET (dialog.dialog));

      conn = vinagre_bookmarks_exists (host, port);
      if (!conn)
	{
	  conn = vinagre_connection_new ();
	  vinagre_connection_set_host (conn, host);
	  vinagre_connection_set_port (conn, port);
	}
    }

  gtk_widget_destroy (dialog.dialog);
  g_object_unref (dialog.xml);
  return conn;
}
/* vim: ts=8 */
