/*
 * vinagre-tab.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008 - Jonh Wendell <wendell@bani.com.br>
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

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <glade/glade.h>
#include <gnome-keyring.h>
#include <vncdisplay.h>

#include "vinagre-notebook.h"
#include "vinagre-tab.h"
#include "vinagre-utils.h"
#include "vinagre-prefs.h"
#include "view/autoDrawer.h"

#define VINAGRE_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_TAB, VinagreTabPrivate))

struct _VinagreTabPrivate
{
  GtkWidget         *vnc;
  GtkWidget         *scroll;
  VinagreConnection *conn;
  VinagreNotebook   *nb;
  VinagreWindow     *window;
  gboolean           save_password;
  guint32            keyring_item_id;
  VinagreTabState    state;
  gchar             *clipboard_str;
  GtkWidget         *layout;
  GtkWidget         *toolbar;
  GtkWidget         *ro_button;
  GtkWidget         *scaling_button;
  gboolean           pointer_grab;
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
  PROP_CONN,
  PROP_WINDOW,
};

static guint signals[LAST_SIGNAL] = { 0 };

static gboolean
vinagre_tab_window_state_cb (GtkWidget           *widget,
			     GdkEventWindowState *event,
			     VinagreTab          *tab)
{
  int vnc_w, vnc_h, screen_w, screen_h;
  GdkScreen *screen;
  GtkPolicyType h, v;

  if ((event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) == 0)
    return FALSE;

  vnc_w = vnc_display_get_width (VNC_DISPLAY (tab->priv->vnc));
  vnc_h = vnc_display_get_height (VNC_DISPLAY (tab->priv->vnc));

  screen = gtk_widget_get_screen (GTK_WIDGET (tab));
  screen_w = gdk_screen_get_width (screen);
  screen_h = gdk_screen_get_height (screen);

  if (vnc_w <= screen_w)
    h = GTK_POLICY_NEVER;
  else
    h = GTK_POLICY_AUTOMATIC;
  
  if (vnc_h <= screen_h)
    v = GTK_POLICY_NEVER;
  else
    v = GTK_POLICY_AUTOMATIC;

  if (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN)
    {
      gtk_widget_show (tab->priv->toolbar);
      ViewAutoDrawer_SetActive (VIEW_AUTODRAWER (tab->priv->layout), TRUE);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tab->priv->scroll),
				      h, v);
    }
  else
    {
      gtk_widget_hide (tab->priv->toolbar);
      ViewAutoDrawer_SetActive (VIEW_AUTODRAWER (tab->priv->layout), FALSE);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tab->priv->scroll),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
      vinagre_notebook_show_hide_tabs (tab->priv->nb);
    }

  return FALSE;
}

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
        g_value_set_object (value, tab->priv->conn);
	break;
      case PROP_WINDOW:
        g_value_set_object (value, tab->priv->window);
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
        tab->priv->conn = g_value_get_object (value);
	g_object_set_data (G_OBJECT (tab->priv->conn), VINAGRE_TAB_KEY, tab);
        break;
      case PROP_WINDOW:
        tab->priv->window = g_value_get_object (value);
	g_signal_connect (tab->priv->window,
			  "window-state-event",
			  G_CALLBACK (vinagre_tab_window_state_cb),
			  tab);
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

  g_signal_handlers_disconnect_by_func (tab->priv->window,
  					vinagre_tab_window_state_cb,
  					tab);
  g_object_unref (tab->priv->conn);
  tab->priv->conn = NULL;

  g_free (tab->priv->clipboard_str);
  tab->priv->clipboard_str = NULL;

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
				   g_param_spec_object ("conn",
							"Connection",
							"The connection",
							VINAGRE_TYPE_CONNECTION,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_NAME |
							G_PARAM_STATIC_NICK |
							G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
				   PROP_WINDOW,
				   g_param_spec_object ("window",
							"Window",
							"The VinagreWindow",
							VINAGRE_TYPE_WINDOW,
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

static void
open_vnc (VinagreTab *tab)
{
  gchar *port;

  port = g_strdup_printf ("%d", vinagre_connection_get_port (tab->priv->conn));
  
  if (!vnc_display_open_host (VNC_DISPLAY(tab->priv->vnc), vinagre_connection_get_host (tab->priv->conn), port))
    vinagre_utils_show_error (_("Error connecting to host."), NULL);

  vnc_display_set_pointer_local (VNC_DISPLAY(tab->priv->vnc), TRUE);
  vnc_display_set_keyboard_grab (VNC_DISPLAY(tab->priv->vnc), TRUE);
  vnc_display_set_pointer_grab (VNC_DISPLAY(tab->priv->vnc), TRUE);

  g_free (port);
  gtk_widget_grab_focus (tab->priv->vnc);
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
  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (tab));

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
  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (tab));

  g_string_printf (message, _("Authentication method to host \"%s\" is unsupported. (%u)"),
		   name,
		   auth_type);

  vinagre_utils_show_error (message->str, GTK_WINDOW (tab->priv->window));
  g_string_free (message, TRUE);
  g_free (name);

  vinagre_notebook_remove_tab (tab->priv->nb, tab);
}

/* text was actually requested */
static void
copy_cb (GtkClipboard     *clipboard,
         GtkSelectionData *data,
	 guint             info,
	 VinagreTab       *tab)
{
  gtk_selection_data_set_text (data, tab->priv->clipboard_str, -1);
}

static void
vnc_server_cut_text_cb (VncDisplay *vnc, const gchar *text, VinagreTab *tab)
{
  GtkClipboard *cb;
  gsize a, b;
  GtkTargetEntry targets[] = {
				{"UTF8_STRING", 0, 0},
				{"COMPOUND_TEXT", 0, 0},
				{"TEXT", 0, 0},
				{"STRING", 0, 0},
			     };

  if (!text)
    return;

  g_free (tab->priv->clipboard_str);
  tab->priv->clipboard_str = g_convert (text, -1, "utf-8", "iso8859-1", &a, &b, NULL);

  if (tab->priv->clipboard_str)
    {
      cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);

      gtk_clipboard_set_with_owner (cb,
				    targets,
				    G_N_ELEMENTS(targets),
				    (GtkClipboardGetFunc) copy_cb,
				    NULL,
				    G_OBJECT (tab));
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
  g_string_printf (uri, "vnc://%s:%d",
                   vinagre_connection_get_host (tab->priv->conn),
                   vinagre_connection_get_port (tab->priv->conn));

  data->display_name = vinagre_connection_get_best_name (tab->priv->conn);
  data->description = NULL;
  data->mime_type = g_strdup ("application/x-remote-connection");
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
                vinagre_connection_get_host (tab->priv->conn),   /* server          */
                NULL,           /* object          */
                "rfb",          /* protocol        */
                "vnc-password", /* authtype        */
                vinagre_connection_get_port (tab->priv->conn),           /* port            */
                vinagre_connection_get_password (tab->priv->conn),       /* password        */
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
		vinagre_connection_get_host (tab->priv->conn),   /* server   */
		NULL,           /* object   */
		"rfb",          /* protocol */
		"vnc-password", /* authtype */
		vinagre_connection_get_port (tab->priv->conn),           /* port     */
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

  name = vinagre_connection_get_best_name (tab->priv->conn);
  label = g_object_get_data (G_OBJECT (tab), "label");
  g_return_if_fail (label != NULL);
  gtk_label_set_label (label, name);
  g_free (name);

  vinagre_window_set_title (tab->priv->window);
  vinagre_tab_save_password (tab);
  vinagre_tab_add_recent_used (tab);

  tab->priv->state = VINAGRE_TAB_STATE_CONNECTED;
  vinagre_window_update_machine_menu_sensitivity (tab->priv->window);

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
  name = vinagre_connection_get_best_name (tab->priv->conn);
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
  int i;

  if (credList == NULL)
    return;

  for (i = 0; i < credList->n_values; i++) {
    switch (g_value_get_enum (&credList->values[i]))
      {
        case VNC_DISPLAY_CREDENTIAL_PASSWORD:
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
          break;

        case VNC_DISPLAY_CREDENTIAL_CLIENTNAME:
          vnc_display_set_credential (vnc, VNC_DISPLAY_CREDENTIAL_CLIENTNAME, "vinagre");
          break;
      }
  }
}

static void
vnc_pointer_grab_cb (VncDisplay *vnc, VinagreTab *tab)
{
  tab->priv->pointer_grab = TRUE;
  vinagre_window_set_title (tab->priv->window);
}

static void
vnc_pointer_ungrab_cb (VncDisplay *vnc, VinagreTab *tab)
{
  tab->priv->pointer_grab = FALSE;
  vinagre_window_set_title (tab->priv->window);
}

static void vnc_bell_cb (VncDisplay *vnc, VinagreTab *tab)
{
  gdk_window_beep (GTK_WIDGET (tab->priv->window)->window);
}

static gboolean
vnc_key_event ( GtkWidget   *widget G_GNUC_UNUSED,
		GdkEventKey *key G_GNUC_UNUSED,
		gpointer     data G_GNUC_UNUSED)
{
  return TRUE;
}

static gboolean
vnc_scroll_event (GtkWidget *widget G_GNUC_UNUSED,
		  GdkEventScroll *scroll G_GNUC_UNUSED,
		  gpointer data G_GNUC_UNUSED)
{
  return TRUE;
}

static void
close_button_clicked (GtkToolButton *button,
		      VinagreTab    *tab)
{
  vinagre_notebook_remove_tab (tab->priv->nb, tab);
}

static void
fullscreen_button_clicked (GtkToolButton *button,
			   VinagreTab    *tab)
{
  vinagre_window_toggle_fullscreen (tab->priv->window);
}

static void
screenshot_button_clicked (GtkToolButton *button,
			   VinagreTab    *tab)
{
  vinagre_tab_take_screenshot (tab);
}

static void
cad_button_clicked (GtkToolButton *button,
			   VinagreTab    *tab)
{
  vinagre_tab_send_ctrlaltdel (tab);
}

static void
ro_button_clicked (GtkToggleToolButton *button,
		   VinagreTab          *tab)
{
  GtkAction *action;

  vinagre_tab_set_readonly (tab, gtk_toggle_tool_button_get_active (button));

  action = gtk_action_group_get_action (vinagre_window_get_connected_action (tab->priv->window), "ViewReadOnly");
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
				vinagre_tab_get_readonly (tab));
}

static void
scaling_button_clicked (GtkToggleToolButton *button,
			VinagreTab          *tab)
{
  GtkAction *action;

  if (!vinagre_tab_set_scaling (tab, gtk_toggle_tool_button_get_active (button)))
    gtk_toggle_tool_button_set_active (button, FALSE);

  action = gtk_action_group_get_action (vinagre_window_get_connected_action (tab->priv->window), "ViewScaling");
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
				vinagre_tab_get_scaling (tab));
}

static void
setup_layout (VinagreTab *tab)
{
  GtkWidget  *button;

  tab->priv->toolbar = gtk_toolbar_new ();
  GTK_WIDGET_SET_FLAGS (tab->priv->toolbar, GTK_NO_SHOW_ALL);

  gtk_toolbar_set_tooltips (GTK_TOOLBAR (tab->priv->toolbar), TRUE);
  gtk_toolbar_set_style (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOLBAR_BOTH_HORIZ);

  /* Leave fullscreen */
  button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_LEAVE_FULLSCREEN));
  gtk_widget_show (GTK_WIDGET (button));
  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (button), TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);
  g_signal_connect (button, "clicked", G_CALLBACK (fullscreen_button_clicked), tab);

  /* Close connection */
  button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_CLOSE));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Close connection"));
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);
  g_signal_connect (button, "clicked", G_CALLBACK (close_button_clicked), tab);

  /* Space */
  button = GTK_WIDGET (gtk_separator_tool_item_new ());
  gtk_separator_tool_item_set_draw (GTK_SEPARATOR_TOOL_ITEM (button), FALSE);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (button), TRUE);
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);

  /* Scaling */
  button = GTK_WIDGET (gtk_toggle_tool_button_new ());
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), _("Scaling"));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Scaling"));
  gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "zoom-fit-best");
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);
  g_signal_connect (button, "toggled", G_CALLBACK (scaling_button_clicked), tab);
  tab->priv->scaling_button = button;

  /* Read only */
  button = GTK_WIDGET (gtk_toggle_tool_button_new ());
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), _("Read only"));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Read only"));
  gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "emblem-readonly");
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);
  g_signal_connect (button, "toggled", G_CALLBACK (ro_button_clicked), tab);
  tab->priv->ro_button = button;

  /* Screenshot */
  button = GTK_WIDGET (gtk_tool_button_new (NULL, NULL));
  gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "applets-screenshooter");
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), _("Take screenshot"));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Take screenshot"));
  g_signal_connect (button, "clicked", G_CALLBACK (screenshot_button_clicked), tab);
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);

  /* Send Ctrl-alt-del */
  button = GTK_WIDGET (gtk_tool_button_new (NULL, NULL));
  gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "preferences-desktop-keyboard-shortcuts");
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), _("Send Ctrl-Alt-Del"));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Send Ctrl-Alt-Del"));
  g_signal_connect (button, "clicked", G_CALLBACK (cad_button_clicked), tab);
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);

  tab->priv->layout = ViewAutoDrawer_New ();
  ViewAutoDrawer_SetActive (VIEW_AUTODRAWER (tab->priv->layout), FALSE);
  ViewOvBox_SetOver (VIEW_OV_BOX (tab->priv->layout), tab->priv->toolbar);
  ViewOvBox_SetUnder (VIEW_OV_BOX (tab->priv->layout), tab->priv->scroll);
}

static void
vinagre_tab_init (VinagreTab *tab)
{
  GtkWidget     *viewport;
  gboolean       shared;

  tab->priv = VINAGRE_TAB_GET_PRIVATE (tab);
  tab->priv->save_password = FALSE;
  tab->priv->keyring_item_id = 0;
  tab->priv->state = VINAGRE_TAB_STATE_INITIALIZING;
  tab->priv->clipboard_str = NULL;
  tab->priv->pointer_grab = FALSE;

  /* Create the scrolled window */
  tab->priv->scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tab->priv->scroll),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (tab->priv->scroll),
				       GTK_SHADOW_NONE);

  /* Create the vnc widget */
  tab->priv->vnc = vnc_display_new ();

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (tab->priv->scroll),
					 tab->priv->vnc);
  viewport = gtk_bin_get_child (GTK_BIN (tab->priv->scroll));
  gtk_viewport_set_shadow_type(GTK_VIEWPORT (viewport), GTK_SHADOW_NONE);

  g_object_get (vinagre_prefs_get_default (),
		"shared-flag", &shared,
		NULL);
  vnc_display_set_shared_flag (VNC_DISPLAY (tab->priv->vnc),
			       shared);

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
		    G_CALLBACK (vnc_pointer_grab_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-pointer-ungrab",
		    G_CALLBACK (vnc_pointer_ungrab_cb),
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

  g_signal_connect (tab->priv->vnc,
		    "key-press-event",
		    G_CALLBACK (vnc_key_event),
		    NULL);

  g_signal_connect (tab->priv->vnc,
		    "key-release-event",
		    G_CALLBACK (vnc_key_event),
		    NULL);

  g_signal_connect (tab->priv->vnc,
		    "scroll-event",
		    G_CALLBACK (vnc_scroll_event),
		    NULL);

  setup_layout (tab);

  gtk_box_pack_end (GTK_BOX(tab), tab->priv->layout, TRUE, TRUE, 0);
  gtk_widget_show_all (GTK_WIDGET (tab));
}

GtkWidget *
vinagre_tab_new (VinagreConnection *conn, VinagreWindow *window)
{
  VinagreTab *tab = g_object_new (VINAGRE_TYPE_TAB, 
				   "conn", conn,
				   "window", window,
				   NULL);
  open_vnc (tab);
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
				  _("Host:"), vinagre_connection_get_host (tab->priv->conn),
				  _("Port:"), vinagre_connection_get_port (tab->priv->conn),
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
  name = vinagre_connection_get_best_name (tab->priv->conn);
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
vinagre_tab_send_ctrlaltdel (VinagreTab *tab)
{
  guint keys[] = { GDK_Control_L, GDK_Alt_L, GDK_Delete };

  g_return_if_fail (VINAGRE_IS_TAB (tab));

  vnc_display_send_keys_ex (VNC_DISPLAY (tab->priv->vnc), keys, sizeof (keys) / sizeof (keys[0]), VNC_DISPLAY_KEY_EVENT_CLICK);
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

gboolean
vinagre_tab_set_scaling (VinagreTab *tab, gboolean active) {
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  if (vnc_display_get_scaling (VNC_DISPLAY (tab->priv->vnc)) == active)
    return TRUE;

  if (active &&
      gdk_screen_is_composited (gtk_widget_get_screen (GTK_WIDGET (tab->priv->window))))
    {
      vinagre_utils_show_error (_("Scaling does not work properly on composited windows. Disable the visual effects and try again."),
				GTK_WINDOW (tab->priv->window));
      return FALSE;
    }

  if (!vnc_display_set_scaling (VNC_DISPLAY (tab->priv->vnc), active))
    {
      vinagre_utils_show_error (_("Scaling is not supported on this installation.\n\nRead the README file (shipped with Vinagre) in order to know how to enable this feature."),
				GTK_WINDOW (tab->priv->window));
      return FALSE;
    }

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (tab->priv->scaling_button), active);
  return TRUE;
}

gboolean
vinagre_tab_get_scaling (VinagreTab *tab) {
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  return vnc_display_get_scaling (VNC_DISPLAY (tab->priv->vnc));
}

void
vinagre_tab_set_readonly (VinagreTab *tab, gboolean active) {
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  vnc_display_set_read_only (VNC_DISPLAY (tab->priv->vnc), active);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (tab->priv->ro_button), active);
}

gboolean
vinagre_tab_get_readonly (VinagreTab *tab) {
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  return vnc_display_get_read_only (VNC_DISPLAY (tab->priv->vnc));
}

VinagreTabState
vinagre_tab_get_state (VinagreTab *tab)
{
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  return tab->priv->state;
}

VinagreTab *
vinagre_tab_get_from_connection (VinagreConnection *conn)
{
  gpointer res;
	
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);
	
  res = g_object_get_data (G_OBJECT (conn), VINAGRE_TAB_KEY);
	
  return (res != NULL) ? VINAGRE_TAB (res) : NULL;
}

gboolean
vinagre_tab_is_pointer_grab (VinagreTab *tab) {
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  return tab->priv->pointer_grab;
}

/* vim: ts=8 */
