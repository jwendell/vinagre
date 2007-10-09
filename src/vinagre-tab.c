/*
 * vinagre-tab.c
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
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <glade/glade.h>
#include <gnome-keyring.h>
#include <vncdisplay.h>

#include "vinagre-notebook.h"
#include "vinagre-tab.h"
#include "vinagre-utils.h"

#define VINAGRE_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_TAB, VinagreTabPrivate))

struct _VinagreTabPrivate
{
  GtkWidget         *vnc;
  VinagreConnection *conn;
  VinagreNotebook   *nb;
  VinagreWindow     *window;
  GtkStatusbar      *status;
  guint              status_id;
  gboolean           save_password;
  guint32            keyring_item_id;
};

G_DEFINE_TYPE(VinagreTab, vinagre_tab, GTK_TYPE_VBOX)

/* Signals */
enum
{
  TAB_CONNECTED,
  TAB_DISCONNECTED,
  TAB_INITIALIZED,
  LAST_SIGNAL
};

/* Properties */
enum
{
  PROP_0,
  PROP_CONN
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
vinagre_tab_get_property (GObject    *object,
			  guint       prop_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
  VinagreTab *tab = VINAGRE_TAB (object);

  switch (prop_id)
    {
      case PROP_CONN:
        g_value_set_pointer (value, tab->priv->conn);
	break;
      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;			
    }
}

static void
vinagre_tab_set_property (GObject      *object,
			  guint         prop_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
  VinagreTab *tab = VINAGRE_TAB (object);

  switch (prop_id)
    {
      case PROP_CONN:
        tab->priv->conn = g_value_get_pointer (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;			
    }
}

static void
vinagre_tab_finalize (GObject *object)
{
  VinagreTab *tab = VINAGRE_TAB (object);

  vinagre_connection_free (tab->priv->conn);
	
  G_OBJECT_CLASS (vinagre_tab_parent_class)->finalize (object);
}

static void 
vinagre_tab_class_init (VinagreTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = vinagre_tab_finalize;
  object_class->get_property = vinagre_tab_get_property;
  object_class->set_property = vinagre_tab_set_property;

  g_object_class_install_property (object_class,
				   PROP_CONN,
				   g_param_spec_pointer ("conn",
							 "Connection",
							 "The connection",
							 G_PARAM_READWRITE |
							 G_PARAM_CONSTRUCT_ONLY |
							 G_PARAM_STATIC_NAME |
							 G_PARAM_STATIC_NICK |
							 G_PARAM_STATIC_BLURB));

  signals[TAB_CONNECTED] =
		g_signal_new ("tab-connected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreTabClass, tab_connected),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

  signals[TAB_DISCONNECTED] =
		g_signal_new ("tab-disconnected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreTabClass, tab_disconnected),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

  signals[TAB_INITIALIZED] =
		g_signal_new ("tab-initialized",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreTabClass, tab_initialized),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

  g_type_class_add_private (object_class, sizeof (VinagreTabPrivate));
}

static gboolean
open_vnc (VinagreTab *tab)
{
  gchar *port;

  tab->priv->status = GTK_STATUSBAR (vinagre_window_get_statusbar (tab->priv->window));
  tab->priv->status_id = gtk_statusbar_get_context_id (tab->priv->status, "VNC Widget Messages");
  
  port = g_strdup_printf ("%d", tab->priv->conn->port);
  
  if (!vnc_display_open_host (VNC_DISPLAY(tab->priv->vnc), tab->priv->conn->host, port))
    vinagre_utils_show_error (_("Error connecting to host."), NULL);

  vnc_display_set_keyboard_grab (VNC_DISPLAY(tab->priv->vnc), TRUE);
  vnc_display_set_pointer_grab  (VNC_DISPLAY(tab->priv->vnc), TRUE);
  vnc_display_set_pointer_local (VNC_DISPLAY(tab->priv->vnc), TRUE);

  g_free (port);
  gtk_widget_grab_focus (tab->priv->vnc);
  return FALSE;
}

static void
vnc_connected_cb (VncDisplay *vnc, VinagreTab *tab)
{
  /* Emits the signal saying that we have connected to the machine */
  g_signal_emit (G_OBJECT (tab),
		 signals[TAB_CONNECTED],
		 0);
}

static void
vnc_disconnected_cb (VncDisplay *vnc, VinagreTab *tab)
{
  /* Emits the signal saying that we have disconnected from the machine */
  g_signal_emit (G_OBJECT (tab),
		 signals[TAB_DISCONNECTED],
		 0);
}

static void
vnc_auth_failed_cb (VncDisplay *vnc, const gchar *msg, VinagreTab *tab)
{
  GString *message;
  gchar   *name;

  message = g_string_new (NULL);
  name = vinagre_connection_best_name (vinagre_tab_get_conn (tab));

  g_string_printf (message, _("Authentication to host \"%s\" has failed"),
		   name);
  if (msg)
  	g_string_append_printf (message, " (%s)", msg);
  g_string_append_printf (message, ".");

  vinagre_utils_show_error (message->str, GTK_WINDOW (tab->priv->window));
  g_string_free (message, TRUE);
  g_free (name);

  if (tab->priv->keyring_item_id > 0)
    {
      gnome_keyring_item_delete_sync (NULL, tab->priv->keyring_item_id);
      tab->priv->keyring_item_id = 0;
    }

  vinagre_notebook_remove_tab (tab->priv->nb, tab);
}

static void
vnc_auth_unsupported_cb (VncDisplay *vnc, guint auth_type, VinagreTab *tab)
{
  GString *message;
  gchar   *name;

  message = g_string_new (NULL);
  name = vinagre_connection_best_name (vinagre_tab_get_conn (tab));

  g_string_printf (message, _("Authentication method to host \"%s\" is unsupported. (%u)"),
		   name,
		   auth_type);

  vinagre_utils_show_error (message->str, GTK_WINDOW (tab->priv->window));
  g_string_free (message, TRUE);
  g_free (name);

  vinagre_notebook_remove_tab (tab->priv->nb, tab);
}

static void
vnc_server_cut_text_cb (VncDisplay *vnc, const gchar *text, VinagreTab *tab)
{
  GtkClipboard *cb;
  gchar *out;
  gsize a, b;

  if (!text)
    return;

  out = g_convert (text, -1, "utf-8", "iso8859-1", &a, &b, NULL);
  if (out) {
    cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text (cb, out, -1);
    g_free (out);
  }
}

static void
vinagre_tab_add_recent_used (VinagreTab *tab)
{
  GtkRecentManager *manager;
  GtkRecentData    *data;
  GString          *uri;

  static gchar *groups[2] = {
		"vinagre",
		NULL
	};

  manager = gtk_recent_manager_get_default ();
  data = g_slice_new (GtkRecentData);

  uri = g_string_new (NULL);
  g_string_printf (uri, "%s:%d", tab->priv->conn->host, tab->priv->conn->port);

  data->display_name = vinagre_connection_best_name (tab->priv->conn);
  data->description = NULL;
  data->mime_type = g_strdup ("application/x-vnc");
  data->app_name = (gchar *) g_get_application_name ();
  data->app_exec = g_strjoin (" ", g_get_prgname (), "%u", NULL);
  data->groups = groups;
  data->is_private = FALSE;

  if (!gtk_recent_manager_add_full (manager, uri->str, data))
    vinagre_utils_show_error (_("Error saving recent connection."),
			      GTK_WINDOW (tab->priv->window));

  g_string_free (uri, TRUE);
  g_free (data->app_exec);
  g_free (data->mime_type);
  g_free (data->display_name);
  g_slice_free (GtkRecentData, data);
}

static void
vinagre_tab_save_password (VinagreTab *tab)
{
  GnomeKeyringResult result;

  if (!tab->priv->save_password)
    return;

  result = gnome_keyring_set_network_password_sync (
                NULL,           /* default keyring */
                NULL,           /* user            */
                NULL,           /* domain          */
                tab->priv->conn->host,   /* server          */
                NULL,           /* object          */
                "rfb",          /* protocol        */
                "vnc-password", /* authtype        */
                tab->priv->conn->port,           /* port            */
                tab->priv->conn->password,       /* password        */
                &tab->priv->keyring_item_id);

  if (result != GNOME_KEYRING_RESULT_OK)
    vinagre_utils_show_error (_("Error saving the password on the keyring."),
			      GTK_WINDOW (tab->priv->window));

  tab->priv->save_password = FALSE;
}

static gchar *
vinagre_tab_find_password (VinagreTab *tab)
{
  GnomeKeyringNetworkPasswordData *found_item;
  GnomeKeyringResult               result;
  GList                           *matches;
  gchar                           *password;
  
  matches = NULL;

  result = gnome_keyring_find_network_password_sync (
                NULL,           /* user     */
		NULL,           /* domain   */
		tab->priv->conn->host,   /* server   */
		NULL,           /* object   */
		"rfb",          /* protocol */
		"vnc-password", /* authtype */
		tab->priv->conn->port,           /* port     */
		&matches);

  if (result != GNOME_KEYRING_RESULT_OK || matches == NULL || matches->data == NULL)
    return NULL;

  found_item = (GnomeKeyringNetworkPasswordData *) matches->data;

  password = g_strdup (found_item->password);
  tab->priv->keyring_item_id = found_item->item_id;

  gnome_keyring_network_password_list_free (matches);

  return password;
}

static void
vnc_initialized_cb (VncDisplay *vnc, VinagreTab *tab)
{
  GtkLabel *label;
  gchar    *name;

  vinagre_connection_set_desktop_name (tab->priv->conn,
				       vnc_display_get_name (VNC_DISPLAY (tab->priv->vnc)));

  name = vinagre_connection_best_name (tab->priv->conn);
  label = g_object_get_data (G_OBJECT (tab), "label");
  g_return_if_fail (label != NULL);
  gtk_label_set_label (label, name);
  g_free (name);

  vinagre_window_set_title (tab->priv->window);
  vinagre_tab_save_password (tab);
  vinagre_tab_add_recent_used (tab);

  /* Emits the signal saying that we have connected to the machine */
  g_signal_emit (G_OBJECT (tab),
		 signals[TAB_INITIALIZED],
		 0);
}

static gchar *
ask_password(VinagreTab *tab)
{
  GladeXML   *xml;
  const char *glade_file;
  GtkWidget  *password_dialog, *password_entry, *host_label, *save_password_check;
  gchar      *password = NULL, *name;
  int         result;

  glade_file = vinagre_utils_get_glade_filename ();
  xml = glade_xml_new (glade_file, NULL, NULL);

  password_dialog = glade_xml_get_widget (xml, "password_required_dialog");
  gtk_window_set_transient_for (GTK_WINDOW(password_dialog), GTK_WINDOW(tab->priv->window));

  host_label = glade_xml_get_widget (xml, "host_label");
  name = vinagre_connection_best_name (tab->priv->conn);
  gtk_label_set_text (GTK_LABEL (host_label), name);
  g_free (name);

  result = gtk_dialog_run (GTK_DIALOG (password_dialog));
  if (result == -5)
    {
      password_entry = glade_xml_get_widget (xml, "password_entry");
      password = g_strdup (gtk_entry_get_text (GTK_ENTRY (password_entry)));

      save_password_check = glade_xml_get_widget (xml, "save_password_check");
      tab->priv->save_password = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (save_password_check));
    }

  gtk_widget_destroy (GTK_WIDGET (password_dialog));
  g_object_unref (xml);

  return password;
}

static void
vnc_authentication_cb (VncDisplay *vnc, GValueArray *credList, VinagreTab *tab)
{
  gchar *password;

  password = vinagre_tab_find_password (tab);
  if (!password)
    {
      password = ask_password (tab);
      if (!password) {
        vinagre_notebook_remove_tab (tab->priv->nb, tab);
        return;
      }
    }

  vinagre_connection_set_password (tab->priv->conn, password);
  vnc_display_set_credential (vnc, VNC_DISPLAY_CREDENTIAL_PASSWORD, password);

  g_free (password);
}

static void vnc_grab_cb (VncDisplay *vnc, VinagreTab *tab)
{
  gtk_statusbar_push (tab->priv->status, tab->priv->status_id, _("Press Ctrl+Alt to release the cursor"));

  if (vinagre_window_is_fullscreen (tab->priv->window))
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (tab->priv->nb), FALSE);
}

static void vnc_bell_cb (VncDisplay *vnc, VinagreTab *tab)
{
  gdk_window_beep (GTK_WIDGET (tab->priv->window)->window);
}

static void vnc_ungrab_cb (VncDisplay *vnc, VinagreTab *tab)
{
  gtk_statusbar_pop (tab->priv->status, tab->priv->status_id);

  if (vinagre_window_is_fullscreen (tab->priv->window))
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (tab->priv->nb), TRUE);
}

static void
vinagre_tab_init (VinagreTab *tab)
{
  GtkWidget *scroll;
  GtkWidget *align;

  tab->priv = VINAGRE_TAB_GET_PRIVATE (tab);
  tab->priv->save_password = FALSE;
  tab->priv->keyring_item_id = 0;

  /* Create the alignment */
  align = gtk_alignment_new (0.5, 0.5, 0, 0);

  /* Create the scrolled window */
  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll),
				       GTK_SHADOW_IN);

  gtk_box_pack_end (GTK_BOX(tab), scroll, TRUE, TRUE, 0);

  /* Create the vnc widget */
  tab->priv->vnc = vnc_display_new ();
  gtk_container_add (GTK_CONTAINER (align), tab->priv->vnc);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll),
					 align);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll),
				       GTK_SHADOW_IN);

  g_signal_connect (tab->priv->vnc,
		    "vnc-connected",
		    G_CALLBACK (vnc_connected_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-initialized",
		    G_CALLBACK (vnc_initialized_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-disconnected",
		    G_CALLBACK (vnc_disconnected_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-auth-credential",
		    G_CALLBACK (vnc_authentication_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-pointer-grab",
		    G_CALLBACK (vnc_grab_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-pointer-ungrab",
		    G_CALLBACK (vnc_ungrab_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-auth-failure",
		    G_CALLBACK (vnc_auth_failed_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-auth-unsupported",
		    G_CALLBACK (vnc_auth_unsupported_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-server-cut-text",
		    G_CALLBACK (vnc_server_cut_text_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-bell",
		    G_CALLBACK (vnc_bell_cb),
		    tab);

 /* connect VNC */
 /* FIXME: i had to add a timeout because private conn is not available at this time*/
  g_timeout_add (1000,
		 (GSourceFunc) open_vnc,
		 tab);

  gtk_widget_show_all (GTK_WIDGET (tab));
}

GtkWidget *
vinagre_tab_new (VinagreConnection *conn, VinagreWindow *window)
{
  VinagreTab *tab = g_object_new (VINAGRE_TYPE_TAB, 
				   "conn", conn,
				   NULL);
  tab->priv->window = window;
  return GTK_WIDGET (tab);
}


gchar *
vinagre_tab_get_tooltips (VinagreTab *tab)
{
  gchar *tip;

  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  tip =  g_markup_printf_escaped (
				  "<b>%s</b> %s\n\n"
				  "<b>%s</b> %s\n"
				  "<b>%s</b> %d\n"
				  "<b>%s</b> %dx%d",
				  _("Desktop Name:"), vnc_display_get_name (VNC_DISPLAY (tab->priv->vnc)),
				  _("Host:"), tab->priv->conn->host,
				  _("Port:"), tab->priv->conn->port,
				  _("Dimensions:"), vnc_display_get_width (VNC_DISPLAY (tab->priv->vnc)), vnc_display_get_height (VNC_DISPLAY (tab->priv->vnc)));

  return tip;
}

VinagreConnection *
vinagre_tab_get_conn (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return tab->priv->conn;
}

GtkWidget *
vinagre_tab_get_vnc (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return tab->priv->vnc;
}

void
vinagre_tab_set_title (VinagreTab *tab,
		       const char *title)
{
  GtkLabel *label;

  g_return_if_fail (VINAGRE_IS_TAB (tab));

  label = GTK_LABEL (g_object_get_data (G_OBJECT (tab),  "label"));
  gtk_label_set_label (label, title);
}

void
vinagre_tab_set_notebook (VinagreTab      *tab,
		          VinagreNotebook *nb)
{
  g_return_if_fail (VINAGRE_IS_TAB (tab));
  g_return_if_fail (VINAGRE_IS_NOTEBOOK (nb));

  tab->priv->nb = nb;
}

VinagreNotebook *
vinagre_tab_get_notebook (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return tab->priv->nb;
}

void
vinagre_tab_take_screenshot (VinagreTab *tab)
{
  GdkPixbuf     *pix;
  GtkWidget     *dialog;
  GString       *suggested_filename;
  gchar         *filename, *name;
  GtkFileFilter *filter;

  g_return_if_fail (VINAGRE_IS_TAB (tab));

  pix = vnc_display_get_pixbuf (VNC_DISPLAY (tab->priv->vnc));

  filename = NULL;
  name = vinagre_connection_best_name (tab->priv->conn);
  suggested_filename = g_string_new (NULL);
  g_string_printf (suggested_filename, _("Screenshot of %s"), name);
  g_string_append (suggested_filename, ".png");

  dialog = gtk_file_chooser_dialog_new (_("Save Screenshot"),
				      GTK_WINDOW (tab->priv->window),
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), suggested_filename->str);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Supported formats"));
  gtk_file_filter_add_pixbuf_formats (filter);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      gdk_pixbuf_save (pix, filename, "png", NULL, NULL);
      g_free (filename);
  }

  gtk_widget_destroy (dialog);
  gdk_pixbuf_unref (pix);
  g_string_free (suggested_filename, TRUE);
  g_free (name);
}

void
vinagre_tab_paste_text (VinagreTab *tab, const gchar *text)
{
  gchar *out;
  size_t a, b;
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  out = g_convert (text, -1, "iso8859-1", "utf-8", &a, &b, NULL);

  if (out)
    {
      vnc_display_client_cut_text (VNC_DISPLAY (tab->priv->vnc), out);
      g_free (out);
    }
}
