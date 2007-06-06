#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib.h>
#include <glade/glade.h>

#include "vinagre-connect.h"
#include "vinagre-main.h"
#include "vinagre-socket.h"
#include "vinagre-utils.h"
#include "vinagre-favorites.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef VINAGRE_HAVE_AVAHI
#include <avahi-client/client.h>
/*FIXME: Replace for <avahi-ui/avahi-ui.h> in 0.6.18*/
#include "avahi-ui.h"
#endif

GladeXML   *xml;
const char *glade_file;

GtkWidget *dialog;
GtkWidget *host_entry;
GtkWidget *port_entry;
GtkWidget *find_button;

#ifdef VINAGRE_HAVE_AVAHI
static void
vinagre_connect_find_button_cb (GtkButton *button,
				gpointer  user_data)
{
  GtkWidget *d;

  d = aui_service_dialog_new (_("Choose a VNC Server"), GTK_WINDOW(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
  gtk_window_set_transient_for (GTK_WINDOW(d), GTK_WINDOW(dialog));
  aui_service_dialog_set_resolve_service (AUI_SERVICE_DIALOG(d), TRUE);
  aui_service_dialog_set_resolve_host_name (AUI_SERVICE_DIALOG(d), TRUE);
  aui_service_dialog_set_browse_service_types (AUI_SERVICE_DIALOG(d),
					       "_rfb._tcp",
					       NULL);

  if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT)
    {
      gtk_entry_set_text (GTK_ENTRY (host_entry),
			  aui_service_dialog_get_host_name(AUI_SERVICE_DIALOG(d)));

      gtk_spin_button_set_value (GTK_SPIN_BUTTON (port_entry),
				 aui_service_dialog_get_port(AUI_SERVICE_DIALOG(d)));
    }

  gtk_widget_destroy(d);
}
#endif

static GtkWidget *
vinagre_connect_create_window ()
{
  glade_file = vinagre_utils_get_glade_filename ();
  xml = glade_xml_new (glade_file, NULL, NULL);
  dialog = glade_xml_get_widget (xml, "connect_dialog");
  gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(main_window));

  host_entry = glade_xml_get_widget (xml, "host_entry");
  port_entry = glade_xml_get_widget (xml, "port_entry");
  find_button = glade_xml_get_widget (xml, "find_button");

#ifdef VINAGRE_HAVE_AVAHI
  gtk_widget_show (find_button);
  g_signal_connect (find_button,
		    "clicked",
		    G_CALLBACK (vinagre_connect_find_button_cb),
		    NULL);

#endif

  gtk_widget_show_all (dialog);
  return dialog;
}

const gchar *
vinagre_connect_ask_password()
{
  GtkWidget *password_dialog, *password_entry;
  gchar *password;
  int result;

  password_dialog = glade_xml_get_widget (xml, "password_required_dialog");
  gtk_window_set_transient_for (GTK_WINDOW(password_dialog), GTK_WINDOW(main_window));

  result = gtk_dialog_run (GTK_DIALOG (password_dialog));
  if (result != -5)
    {
      gtk_widget_destroy (GTK_WIDGET (password_dialog));
      return NULL;
    }

  password_entry = glade_xml_get_widget (xml, "password_entry");
  password = g_strdup (gtk_entry_get_text (GTK_ENTRY (password_entry)));

  gtk_widget_destroy (GTK_WIDGET (password_dialog));

  return password;
}

VinagreConnection *vinagre_connect ()
{
  VinagreConnection *conn = NULL;
  gint               result;
  const gchar       *host;
  int                port;
  int                sock;

  dialog = vinagre_connect_create_window ();

  do
    {
      result = gtk_dialog_run (GTK_DIALOG (dialog));

      if (result == GTK_RESPONSE_OK)
        {
	  host = gtk_entry_get_text (GTK_ENTRY(host_entry));
	  port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (port_entry));
	  sock = connect_to_host (host, port);

	  if (sock == -1)
            {
	      vinagre_utils_show_error (_("Error connecting to host"), GTK_WINDOW(dialog));
              gtk_widget_grab_focus (host_entry);
            }
        }

    } while ((result == GTK_RESPONSE_OK) && (sock == -1));

  if (result == GTK_RESPONSE_OK)
    {
      gtk_widget_hide (GTK_WIDGET (dialog));
      disconnect (sock);

      conn = vinagre_favorites_exists (host, port);
      if (!conn)
	{
	  conn = vinagre_connection_new ();
	  vinagre_connection_set_host (conn, host);
	  vinagre_connection_set_port (conn, port);
	  vinagre_connection_set_password (conn, vinagre_connect_ask_password());
	  vinagre_connection_connect (conn);
	}
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
  return conn;
}
