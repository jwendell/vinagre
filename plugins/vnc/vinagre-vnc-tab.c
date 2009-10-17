/*
 * vinagre-vnc-tab.c
 * VNC Implementation for VinagreVncTab widget
 * This file is part of vinagre
 *
 * Copyright (C) 2009 - Jonh Wendell <wendell@bani.com.br>
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

#include <glib/gi18n.h>
#include <vncdisplay.h>
#include <gdk/gdkkeysyms.h>

#include <vinagre/vinagre-utils.h>
#include <vinagre/vinagre-prefs.h>

#include "vinagre-vnc-tab.h"
#include "vinagre-vnc-connection.h"

#define VINAGRE_VNC_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_VNC_TAB, VinagreVncTabPrivate))

struct _VinagreVncTabPrivate
{
  GtkWidget  *vnc;
  gboolean   pointer_grab;
  gchar      *clipboard_str;
  GSList     *connected_actions, *initialized_actions;
  GtkWidget  *viewonly_button, *scaling_button;
  GtkAction  *scaling_action, *viewonly_action, *original_size_action;
  gulong     signal_clipboard;
};

G_DEFINE_TYPE (VinagreVncTab, vinagre_vnc_tab, VINAGRE_TYPE_TAB)

/* Properties */
enum
{
  PROP_0,
  PROP_ORIGINAL_WIDTH,
  PROP_ORIGINAL_HEIGHT
};

static void open_vnc (VinagreVncTab *vnc_tab);
static void setup_toolbar (VinagreVncTab *vnc_tab);

static void
vinagre_vnc_tab_get_property (GObject    *object,
			  guint       prop_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
  VinagreVncTab *tab = VINAGRE_VNC_TAB (object);

  switch (prop_id)
    {
      case PROP_ORIGINAL_WIDTH:
        g_value_set_int (value, vinagre_vnc_tab_get_original_width (tab));
	break;
      case PROP_ORIGINAL_HEIGHT:
        g_value_set_int (value, vinagre_vnc_tab_get_original_height (tab));
	break;
      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;			
    }
}

static void
view_scaling_cb (GtkAction *action, VinagreVncTab *vnc_tab)
{
  vinagre_vnc_tab_set_scaling (vnc_tab,
			       gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)));
}

static void
view_viewonly_cb (GtkAction *action, VinagreVncTab *vnc_tab)
{
  vinagre_vnc_tab_set_viewonly (vnc_tab,
				gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)));
}

static void
send_ctrlaltdel_cb (GtkAction *action, VinagreVncTab *vnc_tab)
{
  vinagre_vnc_tab_send_ctrlaltdel (vnc_tab);
}

static void
view_original_size_cb (GtkAction *action, VinagreVncTab *vnc_tab)
{
  vinagre_vnc_tab_original_size (vnc_tab);
}

const static GSList *
vnc_get_connected_actions (VinagreTab *tab)
{
  VinagreVncTab *vnc_tab = VINAGRE_VNC_TAB (tab);

  return vnc_tab->priv->connected_actions;
}

const static GSList *
vnc_get_initialized_actions (VinagreTab *tab)
{
  VinagreVncTab *vnc_tab = VINAGRE_VNC_TAB (tab);

  return vnc_tab->priv->initialized_actions;
}

static gchar *
vnc_tab_get_tooltip (VinagreTab *tab)
{
  VinagreVncTab *vnc_tab = VINAGRE_VNC_TAB (tab);
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  return  g_markup_printf_escaped (
				  "<b>%s</b> %s\n\n"
				  "<b>%s</b> %s\n"
				  "<b>%s</b> %d\n"
				  "<b>%s</b> %dx%d",
				  _("Desktop Name:"), vnc_display_get_name (VNC_DISPLAY (vnc_tab->priv->vnc)),
				  _("Host:"), vinagre_connection_get_host (conn),
				  _("Port:"), vinagre_connection_get_port (conn),
				  _("Dimensions:"), vnc_display_get_width (VNC_DISPLAY (vnc_tab->priv->vnc)), vnc_display_get_height (VNC_DISPLAY (vnc_tab->priv->vnc)));
}

static GdkPixbuf *
vnc_tab_get_screenshot (VinagreTab *tab)
{
  VinagreVncTab *vnc_tab = VINAGRE_VNC_TAB (tab);

  return vnc_display_get_pixbuf (VNC_DISPLAY (vnc_tab->priv->vnc));
}

static void
vinagre_vnc_tab_finalize (GObject *object)
{
  VinagreVncTab *vnc_tab = VINAGRE_VNC_TAB (object);

  g_free (vnc_tab->priv->clipboard_str);

  G_OBJECT_CLASS (vinagre_vnc_tab_parent_class)->finalize (object);
}

static void
vinagre_vnc_tab_dispose (GObject *object)
{
  VinagreVncTab *vnc_tab = VINAGRE_VNC_TAB (object);

  if (vnc_tab->priv->connected_actions)
    {
      vinagre_tab_free_actions (vnc_tab->priv->connected_actions);
      vnc_tab->priv->connected_actions = NULL;
    }

  if (vnc_tab->priv->initialized_actions)
    {
      vinagre_tab_free_actions (vnc_tab->priv->initialized_actions);
      vnc_tab->priv->initialized_actions = NULL;
    }

  if (vnc_tab->priv->signal_clipboard != 0)
    {
      GtkClipboard  *cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);

      g_signal_handler_disconnect (cb,
				   vnc_tab->priv->signal_clipboard);
      vnc_tab->priv->signal_clipboard = 0;
    }

  G_OBJECT_CLASS (vinagre_vnc_tab_parent_class)->dispose (object);
}

static void
vinagre_vnc_tab_constructed (GObject *object)
{
  VinagreVncTab *vnc_tab = VINAGRE_VNC_TAB (object);

  if (G_OBJECT_CLASS (vinagre_vnc_tab_parent_class)->constructed)
    G_OBJECT_CLASS (vinagre_vnc_tab_parent_class)->constructed (object);

  setup_toolbar (vnc_tab);
  open_vnc (vnc_tab);
}

static void 
vinagre_vnc_tab_class_init (VinagreVncTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  VinagreTabClass* tab_class = VINAGRE_TAB_CLASS (klass);

  object_class->finalize = vinagre_vnc_tab_finalize;
  object_class->dispose = vinagre_vnc_tab_dispose;
  object_class->get_property = vinagre_vnc_tab_get_property;
  object_class->constructed = vinagre_vnc_tab_constructed;

  tab_class->impl_get_tooltip = vnc_tab_get_tooltip;
  tab_class->impl_get_connected_actions = vnc_get_connected_actions;
  tab_class->impl_get_initialized_actions = vnc_get_initialized_actions;
  tab_class->impl_get_screenshot = vnc_tab_get_screenshot;

  g_object_class_install_property (object_class,
				   PROP_ORIGINAL_WIDTH,
				   g_param_spec_int ("original-width",
						     "Original width",
						     "The original width of the remote screen",
						     -1, G_MAXINT, 0,
						      G_PARAM_READABLE |
						      G_PARAM_STATIC_NAME |
						      G_PARAM_STATIC_NICK |
						      G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
				   PROP_ORIGINAL_HEIGHT,
				   g_param_spec_int ("original-height",
						     "Original height",
						     "The original height of the remote screen",
						     -1, G_MAXINT, 0,
						      G_PARAM_READABLE |
						      G_PARAM_STATIC_NAME |
						      G_PARAM_STATIC_NICK |
						      G_PARAM_STATIC_BLURB));

  g_type_class_add_private (object_class, sizeof (VinagreVncTabPrivate));
}

static void
open_vnc (VinagreVncTab *vnc_tab)
{
  gchar      *host, *port_str;
  gint       port, shared;
  gboolean   scaling;
  VncDisplay *vnc = VNC_DISPLAY (vnc_tab->priv->vnc);
  VinagreTab *tab = VINAGRE_TAB (vnc_tab);

  g_object_get (vinagre_tab_get_conn (tab),
		"port", &port,
		"host", &host,
		"scaling", &scaling,
		"shared", &shared,
		NULL);

  port_str = g_strdup_printf ("%d", port);
  if (shared == -1)
    g_object_get (vinagre_prefs_get_default (),
		  "shared-flag", &shared,
		  NULL);

  vnc_display_set_shared_flag (vnc, shared);
  vnc_display_set_force_size (vnc, !scaling);

  if (vnc_display_open_host (vnc, host, port_str))
    gtk_widget_grab_focus (GTK_WIDGET (vnc));
  else
    vinagre_utils_show_error (NULL,
			      _("Error connecting to host."),
			      GTK_WINDOW (vinagre_tab_get_window (tab)));

  g_free (port_str);
  g_free (host);
}

static void
vnc_connected_cb (VncDisplay *vnc, VinagreVncTab *tab)
{
  g_signal_emit_by_name (G_OBJECT (tab), "tab-connected");
}

static void
vnc_disconnected_cb (VncDisplay *vnc, VinagreVncTab *tab)
{
  g_signal_emit_by_name (G_OBJECT (tab), "tab-disconnected");
}

static void
vnc_auth_failed_cb (VncDisplay *vnc, const gchar *msg, VinagreVncTab *vnc_tab)
{
  vinagre_tab_remove_credentials_from_keyring (VINAGRE_TAB (vnc_tab));
  g_signal_emit_by_name (vnc_tab, "tab-auth-failed", msg);
}

static void
vnc_auth_unsupported_cb (VncDisplay *vnc, guint auth_type, VinagreVncTab *vnc_tab)
{
  GString *message;
  gchar   *name, *emphasis;
  VinagreTab *tab = VINAGRE_TAB (vnc_tab);

  message = g_string_new (NULL);
  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (tab));
  emphasis = g_strdup_printf ("<i>%s</i>", name);

  /* Translators: %s is a host name or IP address; %u is a code error (number). */
  g_string_printf (message, _("Authentication method to host %s is unsupported. (%u)"),
		   emphasis,
		   auth_type);

  vinagre_utils_show_error (_("Authentication unsupported"),
			    message->str,
			    GTK_WINDOW (vinagre_tab_get_window (tab)));
  g_string_free (message, TRUE);
  g_free (name);
  g_free (emphasis);

  vinagre_tab_remove_from_notebook (tab);
}

/* text was actually requested */
static void
copy_cb (GtkClipboard     *clipboard,
         GtkSelectionData *data,
	 guint             info,
	 VinagreVncTab    *vnc_tab)
{
  gtk_selection_data_set_text (data, vnc_tab->priv->clipboard_str, -1);
}

static void
vnc_server_cut_text_cb (VncDisplay *vnc, const gchar *text, VinagreVncTab *vnc_tab)
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

  g_free (vnc_tab->priv->clipboard_str);
  vnc_tab->priv->clipboard_str = g_convert (text, -1, "utf-8", "iso8859-1", &a, &b, NULL);

  if (vnc_tab->priv->clipboard_str)
    {
      cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);

      gtk_clipboard_set_with_owner (cb,
				    targets,
				    G_N_ELEMENTS(targets),
				    (GtkClipboardGetFunc) copy_cb,
				    NULL,
				    G_OBJECT (vnc_tab));
    }
}

static void
vnc_initialized_cb (VncDisplay *vnc, VinagreVncTab *vnc_tab)
{
  GtkLabel *label;
  gchar    *name;
  gboolean scaling, view_only, fullscreen;
  VinagreTab *tab = VINAGRE_TAB (vnc_tab);
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  g_object_get (conn,
		"view-only", &view_only,
		"scaling", &scaling,
		"fullscreen", &fullscreen,
		NULL);

  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (vnc_tab->priv->scaling_action), scaling);
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (vnc_tab->priv->viewonly_action), view_only);
  vnc_display_set_pointer_local (VNC_DISPLAY(vnc_tab->priv->vnc), TRUE);
  vnc_display_set_keyboard_grab (VNC_DISPLAY(vnc_tab->priv->vnc), TRUE);
  vnc_display_set_pointer_grab (VNC_DISPLAY(vnc_tab->priv->vnc), TRUE);

  vinagre_vnc_connection_set_desktop_name (VINAGRE_VNC_CONNECTION (conn),
					   vnc_display_get_name (VNC_DISPLAY (vnc_tab->priv->vnc)));

  name = vinagre_connection_get_best_name (conn);
  label = g_object_get_data (G_OBJECT (tab), "label");
  g_return_if_fail (label != NULL);
  gtk_label_set_label (label, name);
  g_free (name);

  vinagre_tab_save_credentials_in_keyring (tab);
  vinagre_tab_add_recent_used (tab);
  vinagre_tab_set_state (tab, VINAGRE_TAB_STATE_CONNECTED);

  g_signal_emit_by_name (G_OBJECT (tab), "tab-initialized");
}

typedef struct {
  GtkWidget *uname, *pw, *button;
} ControlOKButton;

static void
control_ok_button (GtkEditable *entry, ControlOKButton *data)
{
  gboolean enabled = TRUE;

  if (GTK_WIDGET_VISIBLE (data->uname))
    enabled = enabled && gtk_entry_get_text_length (GTK_ENTRY (data->uname)) > 0;

  if (GTK_WIDGET_VISIBLE (data->pw))
    enabled = enabled && gtk_entry_get_text_length (GTK_ENTRY (data->pw)) > 0;

  gtk_widget_set_sensitive (data->button, enabled);
}

static gboolean
ask_credential (VinagreVncTab *vnc_tab,
		gboolean      need_username,
		gboolean      need_password,
		gchar         **username,
		gchar         **password)
{
  GtkBuilder      *xml;
  GtkWidget       *password_dialog, *host_label, *save_credential_check;
  GtkWidget       *password_label, *username_label, *image;
  gchar           *name;
  int             result;
  ControlOKButton control;
  VinagreTab      *tab = VINAGRE_TAB (vnc_tab);
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  xml = vinagre_utils_get_builder ();

  password_dialog = GTK_WIDGET (gtk_builder_get_object (xml, "auth_required_dialog"));
  gtk_window_set_transient_for (GTK_WINDOW(password_dialog),
				GTK_WINDOW(vinagre_tab_get_window (tab)));

  host_label = GTK_WIDGET (gtk_builder_get_object (xml, "host_label"));
  name = vinagre_connection_get_best_name (conn);
  gtk_label_set_label (GTK_LABEL (host_label), name);
  g_free (name);

  control.uname  = GTK_WIDGET (gtk_builder_get_object (xml, "username_entry"));
  control.pw     = GTK_WIDGET (gtk_builder_get_object (xml, "password_entry"));
  control.button = GTK_WIDGET (gtk_builder_get_object (xml, "ok_button"));
  password_label = GTK_WIDGET (gtk_builder_get_object (xml, "password_label"));
  username_label = GTK_WIDGET (gtk_builder_get_object (xml, "username_label"));
  save_credential_check = GTK_WIDGET (gtk_builder_get_object (xml, "save_credential_check"));

  image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_AUTHENTICATION, GTK_ICON_SIZE_BUTTON);
  gtk_button_set_image (GTK_BUTTON (control.button), image);

  g_signal_connect (control.uname, "changed", G_CALLBACK (control_ok_button), &control);
  g_signal_connect (control.pw, "changed", G_CALLBACK (control_ok_button), &control);

  if (need_username)
    {
      if (*username)
        gtk_entry_set_text (GTK_ENTRY (control.uname), *username);
    }
  else
    {
      gtk_widget_hide (username_label);
      gtk_widget_hide (control.uname);
    }

  if (need_password)
    {
      if (*password)
        gtk_entry_set_text (GTK_ENTRY (control.pw), *password);
    }
  else
    {
      gtk_widget_hide (password_label);
      gtk_widget_hide (control.pw);
    }

  result = gtk_dialog_run (GTK_DIALOG (password_dialog));
  if (result == -5)
    {
      g_free (*username);
      if (gtk_entry_get_text_length (GTK_ENTRY (control.uname)) > 0)
	*username = g_strdup (gtk_entry_get_text (GTK_ENTRY (control.uname)));
      else
	*username = NULL;

      g_free (*password);
      if (gtk_entry_get_text_length (GTK_ENTRY (control.pw)) > 0)
	*password = g_strdup (gtk_entry_get_text (GTK_ENTRY (control.pw)));
      else
	*password = NULL;

      vinagre_tab_set_save_credentials (tab,
					gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (save_credential_check)));
    }

  gtk_widget_destroy (GTK_WIDGET (password_dialog));
  g_object_unref (xml);

  return result == -5;
}

static void
vnc_authentication_cb (VncDisplay *vnc, GValueArray *credList, VinagreVncTab *vnc_tab)
{
  gchar *username, *password;
  gboolean need_password, need_username;
  int i;
  VinagreTab *tab = VINAGRE_TAB (vnc_tab);
  VinagreConnection *conn = vinagre_tab_get_conn (tab);
  VinagreWindow *window = vinagre_tab_get_window (tab);

  if (credList == NULL)
    return;

  need_password = FALSE;
  need_username = FALSE;
  username = NULL;
  password = NULL;

  for (i = 0; i < credList->n_values; i++) {
    switch (g_value_get_enum (&credList->values[i]))
      {
	case VNC_DISPLAY_CREDENTIAL_USERNAME:
	  if (vinagre_connection_get_username (conn))
	    {
	      vnc_display_set_credential (vnc, VNC_DISPLAY_CREDENTIAL_USERNAME, vinagre_connection_get_username (conn));
	      break;
	    }
	  need_username= TRUE;
	  break;

	case VNC_DISPLAY_CREDENTIAL_PASSWORD:
	  if (vinagre_connection_get_password (conn))
	    {
	      vnc_display_set_credential (vnc, VNC_DISPLAY_CREDENTIAL_PASSWORD, vinagre_connection_get_password (conn));
	      break;
	    }
	  need_password = TRUE;
	  break;

        case VNC_DISPLAY_CREDENTIAL_CLIENTNAME:
          vnc_display_set_credential (vnc, VNC_DISPLAY_CREDENTIAL_CLIENTNAME, "vinagre");
          break;
      }
  }

  if (need_password || need_username)
    {
      vinagre_tab_find_credentials_in_keyring (tab, &username, &password);
      if ( (need_username && !username) || (need_password && !password) )
	{
	  if (!ask_credential (vnc_tab, need_username, need_password, &username, &password))
	    {
	      vinagre_tab_remove_from_notebook (tab);
	      goto out;
	    }
	}

      if (need_username)
	{
	  if (username)
	    {
	      vinagre_connection_set_username (conn, username);
	      vnc_display_set_credential (vnc, VNC_DISPLAY_CREDENTIAL_USERNAME, username);
	    }
	  else
	    {
	      vinagre_tab_remove_from_notebook (tab);
	      vinagre_utils_show_error (_("Authentication error"),
					_("A username is required in order to access this machine."),
					GTK_WINDOW (window));
	      goto out;
	    }
	}

      if (need_password)
	{
	  if (password)
	    {
	      vinagre_connection_set_password (conn, password);
	      vnc_display_set_credential (vnc, VNC_DISPLAY_CREDENTIAL_PASSWORD, password);
	    }
	  else
	    {
	      vinagre_tab_remove_from_notebook (tab);
	      vinagre_utils_show_error (_("Authentication error"),
					_("A password is required in order to access this machine."),
					GTK_WINDOW (window));
	      goto out;
	    }
	}

out:
      g_free (username);
      g_free (password);
    }
}

static void
vnc_pointer_grab_cb (VncDisplay *vnc, VinagreVncTab *vnc_tab)
{
  vnc_tab->priv->pointer_grab = TRUE;
}

static void
vnc_pointer_ungrab_cb (VncDisplay *vnc, VinagreVncTab *vnc_tab)
{
  vnc_tab->priv->pointer_grab = FALSE;
}

static void
vnc_bell_cb (VncDisplay *vnc, VinagreVncTab *vnc_tab)
{
  gdk_window_beep (GTK_WIDGET (vnc_tab)->window);
}

static void
vnc_desktop_resize_cb (VncDisplay *vnc, int x, int y, VinagreVncTab *tab)
{
  g_object_notify (G_OBJECT (tab), "original-width");
  g_object_notify (G_OBJECT (tab), "original-height");
  g_object_notify (G_OBJECT (tab), "tooltip");
}

static GSList *
create_connected_actions (VinagreVncTab *tab)
{
  GSList *list = NULL;
  VinagreTabUiAction *a;

  /* View->Scaling */
  a = g_new (VinagreTabUiAction, 1);
  a->paths = g_new (gchar *, 3);
  a->paths[0] = g_strdup ("/MenuBar/ViewMenu");
  a->paths[1] = g_strdup ("/ToolBar");
  a->paths[2] = NULL;
  a->action = GTK_ACTION (gtk_toggle_action_new ("VNCViewScaling",
						 _("S_caling"),
						 _("Fits the remote screen into the current window size"),
						 "zoom-fit-best"));
  gtk_action_set_icon_name (a->action, "zoom-fit-best");
  g_signal_connect (a->action, "activate", G_CALLBACK (view_scaling_cb), tab);
  list = g_slist_append (list, a);
  tab->priv->scaling_action = a->action;

  /* View->View Only */
  a = g_new (VinagreTabUiAction, 1);
  a->paths = g_new (gchar *, 3);
  a->paths[0] = g_strdup ("/MenuBar/ViewMenu");
  a->paths[1] = g_strdup ("/ToolBar");
  a->paths[2] = NULL;
  a->action = GTK_ACTION (gtk_toggle_action_new ("VNCViewViewOnly",
						 _("_View only"),
						 _("Does not send mouse and keyboard events"),
						 "emblem-readonly"));
  gtk_action_set_icon_name (a->action, "emblem-readonly");
  g_signal_connect (a->action, "activate", G_CALLBACK (view_viewonly_cb), tab);
  list = g_slist_append (list, a);
  tab->priv->viewonly_action = a->action;

  /* View->Original Size */
  a = g_new (VinagreTabUiAction, 1);
  a->paths = g_new (gchar *, 2);
  a->paths[0] = g_strdup ("/MenuBar/ViewMenu");
  a->paths[1] = NULL;
  a->action = gtk_action_new ("VNCViewOriginalSize",
			      _("_Original size"),
			      _("Adjusts the window to the remote desktop's size"),
			      "zoom-original");
  gtk_action_set_icon_name (a->action, "zoom-original");
  g_signal_connect (a->action, "activate", G_CALLBACK (view_original_size_cb), tab);
  list = g_slist_append (list, a);
  tab->priv->original_size_action = a->action;

  return list;
}

static GSList *
create_initialized_actions (VinagreVncTab *tab)
{
  GSList *list = NULL;
  VinagreTabUiAction *a;

  /* Machine->Send CTRL-ALT-DEL */
  a = g_new (VinagreTabUiAction, 1);
  a->paths = g_new (gchar *, 3);
  a->paths[0] = g_strdup ("/MenuBar/MachineMenu/MachineOps_1");
  a->paths[1] = g_strdup ("/ToolBar");
  a->paths[2] = NULL;
  a->action = gtk_action_new ("VNCMachineSendCtrlAltDel",
			      _("Send Ctrl-Alt-Del"),
			      _("Sends Ctrl+Alt+Del to the remote machine"),
			      "preferences-desktop-keyboard-shortcuts");
  gtk_action_set_is_important (a->action, TRUE);
  gtk_action_set_icon_name (a->action, "preferences-desktop-keyboard-shortcuts");
  g_signal_connect (a->action, "activate", G_CALLBACK (send_ctrlaltdel_cb), tab);
  list = g_slist_append (list, a);

  return list;
}

static void
viewonly_button_clicked (GtkToggleToolButton *button,
			 VinagreVncTab       *vnc_tab)
{
  vinagre_vnc_tab_set_viewonly (vnc_tab, gtk_toggle_tool_button_get_active (button));

  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (vnc_tab->priv->viewonly_action),
				vinagre_vnc_tab_get_viewonly (vnc_tab));
}

static void
scaling_button_clicked (GtkToggleToolButton *button,
			VinagreVncTab       *vnc_tab)
{
  if (!vinagre_vnc_tab_set_scaling (vnc_tab, gtk_toggle_tool_button_get_active (button)))
    gtk_toggle_tool_button_set_active (button, FALSE);

  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (vnc_tab->priv->scaling_action),
				vinagre_vnc_tab_get_scaling (vnc_tab));
}

static void
cad_button_clicked (GtkToolButton *button,
		    VinagreVncTab *vnc_tab)
{
  vinagre_vnc_tab_send_ctrlaltdel (vnc_tab);
}

static void
setup_toolbar (VinagreVncTab *vnc_tab)
{
  GtkWidget *toolbar = vinagre_tab_get_toolbar (VINAGRE_TAB (vnc_tab));
  GtkWidget *button;

  /* Space */
  button = GTK_WIDGET (gtk_separator_tool_item_new ());
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (button), TRUE);
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (button), -1);

  /* Scaling */
  button = GTK_WIDGET (gtk_toggle_tool_button_new ());
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), _("Scaling"));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Scaling"));
  gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "zoom-fit-best");
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (button), -1);
  g_signal_connect (button, "toggled", G_CALLBACK (scaling_button_clicked), vnc_tab);
  vnc_tab->priv->scaling_button = button;

  /* Read only */
  button = GTK_WIDGET (gtk_toggle_tool_button_new ());
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), _("Read only"));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Read only"));
  gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "emblem-readonly");
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (button), -1);
  g_signal_connect (button, "toggled", G_CALLBACK (viewonly_button_clicked), vnc_tab);
  vnc_tab->priv->viewonly_button = button;

  /* Send Ctrl-alt-del */
  button = GTK_WIDGET (gtk_tool_button_new (NULL, NULL));
  gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "preferences-desktop-keyboard-shortcuts");
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), _("Send Ctrl-Alt-Del"));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Send Ctrl-Alt-Del"));
  g_signal_connect (button, "clicked", G_CALLBACK (cad_button_clicked), vnc_tab);
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (button), -1);
}

static void
vnc_tab_clipboard_cb (GtkClipboard *cb, GdkEvent *event, VinagreVncTab *vnc_tab)
{
  VinagreTab *tab = VINAGRE_TAB (vnc_tab);
  gchar *text;

  if (vinagre_notebook_get_active_tab (vinagre_tab_get_notebook (tab)) != tab)
    return;

  if (VINAGRE_IS_TAB (gtk_clipboard_get_owner (cb)))
    return;

  text = gtk_clipboard_wait_for_text (cb);
  if (!text)
    return;

  vinagre_vnc_tab_paste_text (vnc_tab, text);
  g_free (text);
}

/*
 * Called when the main container widget's size has been set.
 * It attempts to fit the VNC widget into this space while
 * maintaining aspect ratio
 *
 * Code borrowed from from virt-viewer, thanks Daniel Berrange :)
 */
static void
vnc_tab_resize_align (GtkWidget *widget,
		      GtkAllocation *alloc,
		      VinagreVncTab *vnc_tab)
{
  double desktopAspect = (double)vnc_display_get_width (VNC_DISPLAY (vnc_tab->priv->vnc)) / (double)vnc_display_get_height (VNC_DISPLAY (vnc_tab->priv->vnc));
  double scrollAspect = (double)alloc->width / (double)alloc->height;
  int height, width;
  GtkAllocation child;
  int dx = 0, dy = 0;

  if (!vnc_display_is_open (VNC_DISPLAY (vnc_tab->priv->vnc)))
    return;

  if (scrollAspect > desktopAspect)
    {
      width = alloc->height * desktopAspect;
      dx = (alloc->width - width) / 2;
      height = alloc->height;
    }
  else
    {
      width = alloc->width;
      height = alloc->width / desktopAspect;
      dy = (alloc->height - height) / 2;
    }

  child.x = alloc->x + dx;
  child.y = alloc->y + dy;
  child.width = width;
  child.height = height;
  gtk_widget_size_allocate(vnc_tab->priv->vnc, &child);
}

static void
vinagre_vnc_tab_init (VinagreVncTab *vnc_tab)
{
  GtkClipboard *cb;
  GtkWidget *align;

  vnc_tab->priv = VINAGRE_VNC_TAB_GET_PRIVATE (vnc_tab);
  vnc_tab->priv->clipboard_str = NULL;
  vnc_tab->priv->connected_actions = create_connected_actions (vnc_tab);
  vnc_tab->priv->initialized_actions = create_initialized_actions (vnc_tab);

  /* Create the vnc widget */
  vnc_tab->priv->vnc = vnc_display_new ();
  align = gtk_alignment_new (0.5, 0.5, 1, 1);

  g_signal_connect(align, "size-allocate",
		  G_CALLBACK (vnc_tab_resize_align), vnc_tab);
  gtk_container_add (GTK_CONTAINER (align), vnc_tab->priv->vnc);

  vinagre_tab_add_view (VINAGRE_TAB (vnc_tab), align);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-connected",
		    G_CALLBACK (vnc_connected_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-initialized",
		    G_CALLBACK (vnc_initialized_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-disconnected",
		    G_CALLBACK (vnc_disconnected_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-auth-credential",
		    G_CALLBACK (vnc_authentication_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-pointer-grab",
		    G_CALLBACK (vnc_pointer_grab_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-pointer-ungrab",
		    G_CALLBACK (vnc_pointer_ungrab_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-auth-failure",
		    G_CALLBACK (vnc_auth_failed_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-auth-unsupported",
		    G_CALLBACK (vnc_auth_unsupported_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-server-cut-text",
		    G_CALLBACK (vnc_server_cut_text_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-bell",
		    G_CALLBACK (vnc_bell_cb),
		    vnc_tab);

  g_signal_connect (vnc_tab->priv->vnc,
		    "vnc-desktop-resize",
		    G_CALLBACK (vnc_desktop_resize_cb),
		    vnc_tab);

  /* Setup the clipboard */
  cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  vnc_tab->priv->signal_clipboard =  g_signal_connect (cb,
						       "owner-change",
						       G_CALLBACK (vnc_tab_clipboard_cb),
						       vnc_tab);


  gtk_widget_show_all (GTK_WIDGET (vnc_tab));
}

GtkWidget *
vinagre_vnc_tab_new (VinagreConnection *conn,
		     VinagreWindow     *window)
{
  return GTK_WIDGET (g_object_new (VINAGRE_TYPE_VNC_TAB,
				   "conn", conn,
				   "window", window,
				   NULL));
}

void
vinagre_vnc_tab_send_ctrlaltdel (VinagreVncTab *tab)
{
  guint keys[] = { GDK_Control_L, GDK_Alt_L, GDK_Delete };

  g_return_if_fail (VINAGRE_IS_VNC_TAB (tab));

  vnc_display_send_keys_ex (VNC_DISPLAY (tab->priv->vnc), keys, sizeof (keys) / sizeof (keys[0]), VNC_DISPLAY_KEY_EVENT_CLICK);
}

void
vinagre_vnc_tab_paste_text (VinagreVncTab *tab, const gchar *text)
{
  gchar *out;
  gsize a, b;
  GError *error = NULL;

  g_return_if_fail (VINAGRE_IS_VNC_TAB (tab));

  out = g_convert_with_fallback (text, -1, "iso8859-1//TRANSLIT", "utf-8", NULL, &a, &b, &error);
  if (out)
    {
      vnc_display_client_cut_text (VNC_DISPLAY (tab->priv->vnc), out);
      g_free (out);
    }
  else
    {
      g_warning ("%s", error->message);
      g_error_free (error);
    }
}

gboolean
vinagre_vnc_tab_set_scaling (VinagreVncTab *tab, gboolean active) {
  g_return_val_if_fail (VINAGRE_IS_VNC_TAB (tab), FALSE);

  if (vnc_display_get_scaling (VNC_DISPLAY (tab->priv->vnc)) == active)
    return TRUE;

  vnc_display_set_force_size (VNC_DISPLAY(tab->priv->vnc), !active);
  if (!vnc_display_set_scaling (VNC_DISPLAY (tab->priv->vnc), active))
    {
      vinagre_utils_show_error (NULL, _("Scaling is not supported on this installation.\n\nRead the README file (shipped with Vinagre) in order to know how to enable this feature."),
				GTK_WINDOW (vinagre_tab_get_window (VINAGRE_TAB (tab))));
      return FALSE;
    }

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (tab->priv->scaling_button),
				     active);

  if (active)
    gtk_widget_set_size_request (tab->priv->vnc, 0, 0);
  else
    gtk_widget_set_size_request (tab->priv->vnc,
				 vnc_display_get_width (VNC_DISPLAY (tab->priv->vnc)),
				 vnc_display_get_height (VNC_DISPLAY (tab->priv->vnc)));

  return TRUE;
}

gboolean
vinagre_vnc_tab_get_scaling (VinagreVncTab *tab) {
  g_return_val_if_fail (VINAGRE_IS_VNC_TAB (tab), FALSE);

  return vnc_display_get_scaling (VNC_DISPLAY (tab->priv->vnc));
}

void
vinagre_vnc_tab_set_viewonly (VinagreVncTab *tab, gboolean active) {
  g_return_if_fail (VINAGRE_IS_VNC_TAB (tab));

  vnc_display_set_read_only (VNC_DISPLAY (tab->priv->vnc), active);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (tab->priv->viewonly_button),
				     active);

}

gboolean
vinagre_vnc_tab_get_viewonly (VinagreVncTab *tab) {
  g_return_val_if_fail (VINAGRE_IS_VNC_TAB (tab), FALSE);

  return vnc_display_get_read_only (VNC_DISPLAY (tab->priv->vnc));
}

gboolean
vinagre_vnc_tab_is_pointer_grab (VinagreVncTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_VNC_TAB (tab), FALSE);

  return tab->priv->pointer_grab;
}

gint
vinagre_vnc_tab_get_original_height (VinagreVncTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_VNC_TAB (tab), -1);

  if (VNC_IS_DISPLAY (tab->priv->vnc))
    return vnc_display_get_height (VNC_DISPLAY (tab->priv->vnc));
  else
    return -1;
}

gint
vinagre_vnc_tab_get_original_width (VinagreVncTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_VNC_TAB (tab), -1);

  if (VNC_IS_DISPLAY (tab->priv->vnc))
    return vnc_display_get_width (VNC_DISPLAY (tab->priv->vnc));
  else
    return -1;
}


typedef struct _VinagrePrefSize {
  gint width, height;
  gulong sig_id;
} VinagrePrefSize;

static gboolean
cb_unset_size (gpointer data)
{
  GtkWidget *widget = data;

  gtk_widget_queue_resize_no_redraw (widget);

  return FALSE;
}

static void
cb_set_preferred_size (GtkWidget *widget, GtkRequisition *req,
		       gpointer data)
{
  VinagrePrefSize *size = data;

  req->width = size->width;
  req->height = size->height;

  g_signal_handler_disconnect (widget, size->sig_id);
  g_free (size);
  g_idle_add (cb_unset_size, widget);
}

static void
vinagre_widget_set_preferred_size (GtkWidget *widget,
				   gint      width,
				   gint      height)
{
  VinagrePrefSize *size = g_new (VinagrePrefSize, 1);

  size->width  = width;
  size->height = height;
  size->sig_id = g_signal_connect (widget, "size-request",
				   G_CALLBACK (cb_set_preferred_size),
				   size);

  gtk_widget_queue_resize (widget);
}


void
vinagre_vnc_tab_original_size (VinagreVncTab *tab)
{
  GtkWindow *window;
  g_return_if_fail (VINAGRE_IS_VNC_TAB (tab));

  window = GTK_WINDOW (vinagre_tab_get_window (VINAGRE_TAB (tab)));

  gtk_window_unmaximize (window);
  gtk_window_resize (window, 1, 1);
  vinagre_widget_set_preferred_size (GTK_WIDGET (tab),
				     vinagre_vnc_tab_get_original_width (tab),
				     vinagre_vnc_tab_get_original_height (tab));
}

/* vim: set ts=8: */
