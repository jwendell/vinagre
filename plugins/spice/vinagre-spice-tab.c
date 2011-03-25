/*
 * vinagre-spice-tab.c
 * SPICE Implementation for VinagreSpiceTab widget
 * This file is part of vinagre
 *
 * Copyright (C) 2011 Red Hat, Inc.
 * Author: Marc-Andre Lureau <marcandre.lureau@redhat.com>
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

#include <config.h>
#include <glib/gi18n.h>
#include <spice-client-glib-2.0/spice-session.h>
#include <spice-client-glib-2.0/spice-audio.h>
#include <spice-client-gtk-3.0/spice-widget.h>
#include <gdk/gdkkeysyms.h>

#include <vinagre/vinagre-utils.h>
#include <vinagre/vinagre-prefs.h>

#include "vinagre-spice-tab.h"
#include "vinagre-spice-connection.h"
#include "vinagre-spice-tunnel.h"

#define VINAGRE_SPICE_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_SPICE_TAB, VinagreSpiceTabPrivate))

typedef struct _VinagreSpiceDisplay
{
  GtkWidget  *display;
} VinagreSpiceDisplay;

struct _VinagreSpiceTabPrivate
{
  SpiceSession *spice;
  SpiceAudio *audio;
  GtkWidget  *display, *align;
  gboolean   mouse_grabbed;
  GSList     *connected_actions, *initialized_actions;
  GtkWidget  *viewonly_button, *scaling_button;
  GtkAction  *scaling_action, *viewonly_action, *original_size_action, *resize_guest_action, *auto_clipboard_action;
  gulong     signal_align;
  VinagreSpiceDisplay *wins[4]; /* TODO: handle multi-display */
};

G_DEFINE_TYPE (VinagreSpiceTab, vinagre_spice_tab, VINAGRE_TYPE_TAB)

/* Properties */
enum
{
  PROP_0,
};

static void open_spice (VinagreSpiceTab *spice_tab);
static void setup_toolbar (VinagreSpiceTab *spice_tab);

static void
vinagre_spice_tab_get_property (GObject	   *object,
				guint	    prop_id,
				GValue	   *value,
				GParamSpec *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
view_scaling_cb (GtkAction *action, VinagreSpiceTab *spice_tab)
{
  vinagre_spice_tab_set_scaling (spice_tab,
				 gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)));
}

static void
view_resize_guest_cb (GtkAction *action, VinagreSpiceTab *spice_tab)
{
  vinagre_spice_tab_set_resize_guest (spice_tab,
				      gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)));
}

static void
view_auto_clipboard_cb (GtkAction *action, VinagreSpiceTab *spice_tab)
{
  vinagre_spice_tab_set_auto_clipboard (spice_tab,
					gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)));
}

static void
view_viewonly_cb (GtkAction *action, VinagreSpiceTab *spice_tab)
{
  vinagre_spice_tab_set_viewonly (spice_tab,
				  gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)));
}

static void
send_ctrlaltdel_cb (GtkAction *action, VinagreSpiceTab *spice_tab)
{
  vinagre_spice_tab_send_ctrlaltdel (spice_tab);
}

const static GSList *
spice_get_connected_actions (VinagreTab *tab)
{
  VinagreSpiceTab *spice_tab = VINAGRE_SPICE_TAB (tab);

  return spice_tab->priv->connected_actions;
}

const static GSList *
spice_get_initialized_actions (VinagreTab *tab)
{
  VinagreSpiceTab *spice_tab = VINAGRE_SPICE_TAB (tab);

  return spice_tab->priv->initialized_actions;
}

static gchar *
spice_tab_get_tooltip (VinagreTab *tab)
{
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  return  g_markup_printf_escaped ("<b>%s</b> %s\n\n"
				   "<b>%s</b> %d\n",
				   _("Host:"), vinagre_connection_get_host (conn),
				   _("Port:"), vinagre_connection_get_port (conn));
}

static void
vinagre_spice_tab_finalize (GObject *object)
{
  G_OBJECT_CLASS (vinagre_spice_tab_parent_class)->finalize (object);
}

static void
vinagre_spice_tab_dispose (GObject *object)
{
  VinagreSpiceTab *spice_tab = VINAGRE_SPICE_TAB (object);

  if (spice_tab->priv->connected_actions)
    {
      vinagre_tab_free_actions (spice_tab->priv->connected_actions);
      spice_tab->priv->connected_actions = NULL;
    }

  if (spice_tab->priv->initialized_actions)
    {
      vinagre_tab_free_actions (spice_tab->priv->initialized_actions);
      spice_tab->priv->initialized_actions = NULL;
    }

  if (spice_tab->priv->audio)
    {
      g_object_unref (spice_tab->priv->audio);
      spice_tab->priv->audio = NULL;
    }

  if (spice_tab->priv->spice)
    {
      spice_session_disconnect (spice_tab->priv->spice);
      g_object_unref (spice_tab->priv->spice);
      spice_tab->priv->spice = NULL;
    }

  G_OBJECT_CLASS (vinagre_spice_tab_parent_class)->dispose (object);
}

static void
vinagre_spice_tab_constructed (GObject *object)
{
  VinagreSpiceTab *spice_tab = VINAGRE_SPICE_TAB (object);

  if (G_OBJECT_CLASS (vinagre_spice_tab_parent_class)->constructed)
    G_OBJECT_CLASS (vinagre_spice_tab_parent_class)->constructed (object);

  setup_toolbar (spice_tab);
  open_spice (spice_tab);
}

static void
vinagre_spice_tab_class_init (VinagreSpiceTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  VinagreTabClass* tab_class = VINAGRE_TAB_CLASS (klass);

  object_class->finalize = vinagre_spice_tab_finalize;
  object_class->dispose = vinagre_spice_tab_dispose;
  object_class->get_property = vinagre_spice_tab_get_property;
  object_class->constructed = vinagre_spice_tab_constructed;

  tab_class->impl_get_tooltip = spice_tab_get_tooltip;
  tab_class->impl_get_connected_actions = spice_get_connected_actions;
  tab_class->impl_get_initialized_actions = spice_get_initialized_actions;

  g_type_class_add_private (object_class, sizeof (VinagreSpiceTabPrivate));
}

static gboolean
idle_close (VinagreTab *tab)
{
  vinagre_notebook_close_tab (vinagre_tab_get_notebook (tab), tab);
  return FALSE;
}

static void
open_spice (VinagreSpiceTab *spice_tab)
{
  gchar	     *host, *port_str, *ssh_tunnel_host;
  gint	     port, fd;
  gboolean   scaling, success;
  GError     *error;
  SpiceSession *spice = spice_tab->priv->spice;
  VinagreTab *tab = VINAGRE_TAB (spice_tab);
  GtkWindow  *window = GTK_WINDOW (vinagre_tab_get_window (tab));
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  success = TRUE;
  error = NULL;

  g_object_get (conn,
		"port", &port,
		"host", &host,
		"scaling", &scaling,
		"fd", &fd,
		"ssh-tunnel-host", &ssh_tunnel_host,
		NULL);

  port_str = g_strdup_printf ("%d", port);

  if (fd > 0)
    success = spice_session_open_fd (spice, fd);
  else
    {
      if (ssh_tunnel_host && *ssh_tunnel_host)
	if (!vinagre_spice_tunnel_create (window, &host, &port_str, ssh_tunnel_host, &error))
	  {
	    success = FALSE;
	    vinagre_utils_show_error (_("Error creating the SSH tunnel"),
				      error ? error->message : _("Unknown reason"),
				      window);
	    goto out;
	  }
      g_object_set (spice, "host", host, "port", port_str,
		    "password", vinagre_connection_get_password (conn),
		    NULL);
      success = spice_session_connect (spice);
    }

  if (!success)
    vinagre_utils_show_error (_("Error connecting to host."),
			      error ? error->message : _("Unknown reason"),
			      window);

 out:
  g_free (port_str);
  g_free (host);
  g_free (ssh_tunnel_host);
  g_clear_error (&error);

  if (!success)
    g_idle_add ((GSourceFunc)idle_close, spice_tab);
}

static void
spice_main_channel_event_cb(SpiceChannel *channel, SpiceChannelEvent event,
			    VinagreSpiceTab *spice_tab)
{
  switch (event) {
  case SPICE_CHANNEL_OPENED:
    g_signal_emit_by_name (G_OBJECT (spice_tab), "tab-connected");
    break;
  case SPICE_CHANNEL_CLOSED:
    g_signal_emit_by_name (G_OBJECT (spice_tab), "tab-disconnected");
    break;
  case SPICE_CHANNEL_ERROR_AUTH: {
    VinagreTab *tab = VINAGRE_TAB (spice_tab);
    VinagreConnection *conn = vinagre_tab_get_conn (tab);
    gchar *name = vinagre_connection_get_best_name (conn), *password = NULL;
    GtkWindow *window = GTK_WINDOW (vinagre_tab_get_window (tab));
    gboolean save_in_keyring = FALSE;

    vinagre_tab_remove_credentials_from_keyring (VINAGRE_TAB (tab));
    if (!vinagre_utils_ask_credential (window,
				       "SPICE",
				       name,
				       FALSE,
				       TRUE,
				       8,
				       NULL,
				       &password,
				       &save_in_keyring))
      {
	vinagre_tab_remove_from_notebook (tab);
	goto out;
      }
    vinagre_connection_set_password (conn, password);
    vinagre_tab_set_save_credentials (tab, save_in_keyring);

    g_object_set (spice_tab->priv->spice, "password", password, NULL);
    spice_session_connect (spice_tab->priv->spice);

    out:
    g_free (password);
    break;
  }
  case SPICE_CHANNEL_ERROR_IO:
  case SPICE_CHANNEL_ERROR_TLS:
  case SPICE_CHANNEL_ERROR_LINK:
  case SPICE_CHANNEL_ERROR_CONNECT:
    g_signal_emit_by_name (G_OBJECT (spice_tab), "tab-disconnected");
    break;
  default:
    g_warning("unhandled main channel event: %d", event);
    break;
  }
}

static void
spice_mouse_grab_cb(GtkWidget *widget, gint grabbed, VinagreSpiceTab *spice_tab)
{
  spice_tab->priv->mouse_grabbed = grabbed;
}

static VinagreSpiceDisplay *
create_spice_display (VinagreSpiceTab *spice_tab, int id)
{
  VinagreSpiceDisplay *d;
  GtkLabel *label;
  gchar	   *name;
  gboolean scaling, resize_guest, view_only, fullscreen, auto_clipboard;
  VinagreTab *tab = VINAGRE_TAB (spice_tab);
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  d = g_new0(VinagreSpiceDisplay, 1);

  /* Create the display widget */
  spice_tab->priv->display = d->display = GTK_WIDGET (spice_display_new (spice_tab->priv->spice, id));

  vinagre_tab_add_view (tab, d->display);
  vinagre_tab_set_has_screenshot (tab, TRUE);

  g_signal_connect (spice_tab->priv->display, "mouse-grab",
		    G_CALLBACK (spice_mouse_grab_cb), spice_tab);

  g_object_get (conn,
		"fullscreen", &fullscreen,
		"resize-guest", &resize_guest,
		"auto-clipboard", &auto_clipboard,
		"scaling", &scaling,
		"view-only", &view_only,
		NULL);

  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (spice_tab->priv->scaling_action), scaling);
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (spice_tab->priv->resize_guest_action), resize_guest);
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (spice_tab->priv->auto_clipboard_action), auto_clipboard);
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (spice_tab->priv->viewonly_action), view_only);
  g_object_set (d->display,
		"grab-keyboard", TRUE,
		"grab-mouse", TRUE,
		"resize-guest", resize_guest,
		"auto-clipboard", auto_clipboard,
		"scaling", scaling,
		NULL);
  /* TODO: add view-only here when spice-gtk ready */

  g_object_get (spice_tab->priv->spice, "uri", &name, NULL); /* TODO: a better friendly name? */
  vinagre_spice_connection_set_desktop_name (VINAGRE_SPICE_CONNECTION (conn), name);
  g_free (name);

  name = vinagre_connection_get_best_name (conn);
  label = g_object_get_data (G_OBJECT (tab), "label");
  g_return_val_if_fail (label != NULL, d);
  gtk_label_set_label (label, name);
  g_free (name);

  vinagre_tab_save_credentials_in_keyring (tab);
  vinagre_tab_add_recent_used (tab);
  vinagre_tab_set_state (tab, VINAGRE_TAB_STATE_CONNECTED);

  g_signal_emit_by_name (G_OBJECT (tab), "tab-initialized");

  gtk_widget_show_all (GTK_WIDGET (tab));

  return d;
}

static void
destroy_spice_display (VinagreSpiceTab *tab, VinagreSpiceDisplay *display)
{
}

static void
spice_channel_new_cb (SpiceSession *s, SpiceChannel *channel, VinagreSpiceTab *tab)
{
  int id;

  g_object_get (channel, "channel-id", &id, NULL);
  g_object_ref (tab);

  if (SPICE_IS_MAIN_CHANNEL (channel)) {
    g_signal_connect (channel, "channel-event",
		      G_CALLBACK (spice_main_channel_event_cb), tab);
  }

  if (SPICE_IS_DISPLAY_CHANNEL (channel)) {
    if (id >= G_N_ELEMENTS (tab->priv->wins))
      return;
    if (tab->priv->wins[id] != NULL)
      return;
    tab->priv->wins[id] = create_spice_display (tab, id);
  }

  if (SPICE_IS_PLAYBACK_CHANNEL (channel) ||
      SPICE_IS_RECORD_CHANNEL (channel)) {
    if (tab->priv->audio != NULL)
      return;
    tab->priv->audio = spice_audio_new (s, NULL, NULL);
  }
}

static void
spice_channel_destroy_cb (SpiceSession *s, SpiceChannel *channel, VinagreSpiceTab *tab)
{
  int id;

  g_object_get (channel, "channel-id", &id, NULL);
  g_object_unref (tab);

  if (SPICE_IS_DISPLAY_CHANNEL (channel)) {
    if (id >= G_N_ELEMENTS (tab->priv->wins))
      return;
    if (tab->priv->wins[id] == NULL)
      return;
    destroy_spice_display (tab, tab->priv->wins[id]);
    tab->priv->wins[id] = NULL;
  }
}

static GSList *
create_connected_actions (VinagreSpiceTab *tab)
{
  GSList *list = NULL;
  VinagreTabUiAction *a;

  /* View->Scaling */
  a = g_new (VinagreTabUiAction, 1);
  a->paths = g_new (gchar *, 3);
  a->paths[0] = g_strdup ("/MenuBar/ViewMenu");
  a->paths[1] = g_strdup ("/ToolBar");
  a->paths[2] = NULL;
  a->action = GTK_ACTION (gtk_toggle_action_new ("SPICEViewScaling",
						 _("S_caling"),
						 _("Fits the remote screen into the current window size"),
						 "zoom-fit-best"));
  gtk_action_set_icon_name (a->action, "zoom-fit-best");
  g_signal_connect (a->action, "activate", G_CALLBACK (view_scaling_cb), tab);
  list = g_slist_append (list, a);
  tab->priv->scaling_action = a->action;

  /* View->Resize Guest */
  a = g_new (VinagreTabUiAction, 1);
  a->paths = g_new (gchar *, 2);
  a->paths[0] = g_strdup ("/MenuBar/ViewMenu");
  a->paths[1] = NULL;
  a->action = GTK_ACTION (gtk_toggle_action_new ("SPICEViewResizeGuest",
						 _("_Resize guest"),
						 _("Resize the screen guest to best fit"),
						 NULL));
  g_signal_connect (a->action, "activate", G_CALLBACK (view_resize_guest_cb), tab);
  list = g_slist_append (list, a);
  tab->priv->resize_guest_action = a->action;

  /* View->Share clipboard */
  a = g_new (VinagreTabUiAction, 1);
  a->paths = g_new (gchar *, 2);
  a->paths[0] = g_strdup ("/MenuBar/ViewMenu");
  a->paths[1] = NULL;
  a->action = GTK_ACTION (gtk_toggle_action_new ("SPICEViewAutoClipboard",
						 _("_Share clipboard"),
						 _("Automatically share clipboard between client and guest"),
						 NULL));
  g_signal_connect (a->action, "activate", G_CALLBACK (view_auto_clipboard_cb), tab);
  list = g_slist_append (list, a);
  tab->priv->auto_clipboard_action = a->action;

  /* View->View Only TODO: not ready in spice-gtk yet */
  a = g_new (VinagreTabUiAction, 1);
  a->paths = g_new (gchar *, 3);
  a->paths[0] = g_strdup ("/MenuBar/ViewMenu");
  a->paths[1] = g_strdup ("/ToolBar");
  a->paths[2] = NULL;
  a->action = GTK_ACTION (gtk_toggle_action_new ("SPICEViewViewOnly",
						 _("_View only"),
						 _("Does not send mouse and keyboard events"),
						 "emblem-readonly"));
  gtk_action_set_icon_name (a->action, "emblem-readonly");
  g_signal_connect (a->action, "activate", G_CALLBACK (view_viewonly_cb), tab);
  /* list = g_slist_append (list, a); */
  tab->priv->viewonly_action = a->action;

  return list;
}

static GSList *
create_initialized_actions (VinagreSpiceTab *tab)
{
  GSList *list = NULL;
  VinagreTabUiAction *a;

  /* Machine->Send CTRL-ALT-DEL */
  a = g_new (VinagreTabUiAction, 1);
  a->paths = g_new (gchar *, 3);
  a->paths[0] = g_strdup ("/MenuBar/MachineMenu/MachineOps_1");
  a->paths[1] = g_strdup ("/ToolBar");
  a->paths[2] = NULL;
  a->action = gtk_action_new ("SPICEMachineSendCtrlAltDel",
			      _("_Send Ctrl-Alt-Del"),
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
			 VinagreSpiceTab     *spice_tab)
{
  vinagre_spice_tab_set_viewonly (spice_tab, gtk_toggle_tool_button_get_active (button));

  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (spice_tab->priv->viewonly_action),
				vinagre_spice_tab_get_viewonly (spice_tab));
}

static void
scaling_button_clicked (GtkToggleToolButton *button,
			VinagreSpiceTab	    *spice_tab)
{
  if (!vinagre_spice_tab_set_scaling (spice_tab, gtk_toggle_tool_button_get_active (button)))
    gtk_toggle_tool_button_set_active (button, FALSE);

  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (spice_tab->priv->scaling_action),
				vinagre_spice_tab_get_scaling (spice_tab));
}

static void
cad_button_clicked (GtkToolButton *button,
		    VinagreSpiceTab *spice_tab)
{
  vinagre_spice_tab_send_ctrlaltdel (spice_tab);
}

static void
setup_toolbar (VinagreSpiceTab *spice_tab)
{
  GtkWidget *toolbar = vinagre_tab_get_toolbar (VINAGRE_TAB (spice_tab));
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
  g_signal_connect (button, "toggled", G_CALLBACK (scaling_button_clicked), spice_tab);
  spice_tab->priv->scaling_button = button;

  /* Read only */
  button = GTK_WIDGET (gtk_toggle_tool_button_new ());
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), _("Read only"));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Read only"));
  gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "emblem-readonly");
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (button), -1);
  g_signal_connect (button, "toggled", G_CALLBACK (viewonly_button_clicked), spice_tab);
  spice_tab->priv->viewonly_button = button;

  /* Send Ctrl-alt-del */
  button = GTK_WIDGET (gtk_tool_button_new (NULL, _("Send Ctrl-Alt-Del")));
  gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "preferences-desktop-keyboard-shortcuts");
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Send Ctrl-Alt-Del"));
  g_signal_connect (button, "clicked", G_CALLBACK (cad_button_clicked), spice_tab);
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (button), -1);
}

static void
vinagre_spice_tab_init (VinagreSpiceTab *spice_tab)
{
  SpiceSession *spice;

  spice_tab->priv = VINAGRE_SPICE_TAB_GET_PRIVATE (spice_tab);
  spice_tab->priv->connected_actions = create_connected_actions (spice_tab);
  spice_tab->priv->initialized_actions = create_initialized_actions (spice_tab);

  /* Create the spice widget */
  spice = spice_tab->priv->spice = spice_session_new ();
  g_signal_connect(spice, "channel-new",
		   G_CALLBACK(spice_channel_new_cb), spice_tab);
  g_signal_connect(spice, "channel-destroy",
		   G_CALLBACK(spice_channel_destroy_cb), spice_tab);
}

GtkWidget *
vinagre_spice_tab_new (VinagreConnection *conn,
		       VinagreWindow	 *window)
{
  return GTK_WIDGET (g_object_new (VINAGRE_TYPE_SPICE_TAB,
				   "conn", conn,
				   "window", window,
				   NULL));
}

void
vinagre_spice_tab_send_ctrlaltdel (VinagreSpiceTab *tab)
{
  guint keys[] = { GDK_KEY_Control_L, GDK_KEY_Alt_L, GDK_KEY_Delete };

  g_return_if_fail (VINAGRE_IS_SPICE_TAB (tab));

  spice_display_send_keys (SPICE_DISPLAY (tab->priv->display), keys, sizeof (keys) / sizeof (keys[0]), SPICE_DISPLAY_KEY_EVENT_CLICK);
}

gboolean
vinagre_spice_tab_set_scaling (VinagreSpiceTab *tab, gboolean active)
{
  gboolean scaling;

  g_return_val_if_fail (VINAGRE_IS_SPICE_TAB (tab), FALSE);

  g_object_get (tab->priv->display, "scaling", &scaling, NULL);
  if (scaling == active)
    return TRUE;

  g_object_set (tab->priv->display, "scaling", active, NULL);

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (tab->priv->scaling_button),
				     active);

  return TRUE;
}

gboolean
vinagre_spice_tab_get_scaling (VinagreSpiceTab *tab)
{
  gboolean scaling;

  g_return_val_if_fail (VINAGRE_IS_SPICE_TAB (tab), FALSE);

  g_object_get (tab->priv->display, "scaling", &scaling, NULL);

  return scaling;
}

void
vinagre_spice_tab_set_viewonly (VinagreSpiceTab *tab, gboolean active)
{
  g_return_if_fail (VINAGRE_IS_SPICE_TAB (tab));

#if 0
  g_object_set (tab->priv->display, "read-only", active, NULL); /* only in future version of spice-gtk */
#endif

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (tab->priv->viewonly_button),
				     active);
}

gboolean
vinagre_spice_tab_get_viewonly (VinagreSpiceTab *tab)
{
  gboolean active = FALSE;

  g_return_val_if_fail (VINAGRE_IS_SPICE_TAB (tab), FALSE);
#if 0
  g_object_get (tab->priv->display, "read-only", &active, NULL);  /* only in future version of spice-gtk */
#endif
  return active;
}

gboolean
vinagre_spice_tab_set_resize_guest (VinagreSpiceTab *tab, gboolean active)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_TAB (tab), FALSE);

  g_object_set (tab->priv->display, "resize-guest", active, NULL);

  return TRUE;
}

gboolean
vinagre_spice_tab_get_resize_guest (VinagreSpiceTab *tab)
{
  gboolean active;

  g_return_val_if_fail (VINAGRE_IS_SPICE_TAB (tab), FALSE);

  g_object_get (tab->priv->display, "resize-guest", &active, NULL);

  return active;
}

void
vinagre_spice_tab_set_auto_clipboard (VinagreSpiceTab *tab, gboolean active)
{
  g_return_if_fail (VINAGRE_IS_SPICE_TAB (tab));

  g_object_set (tab->priv->display, "auto-clipboard", active, NULL);
}

gboolean
vinagre_spice_tab_get_auto_clipboard (VinagreSpiceTab *tab)
{
  gboolean active;

  g_return_val_if_fail (VINAGRE_IS_SPICE_TAB (tab), FALSE);

  g_object_get (tab->priv->display, "auto-clipboard", &active, NULL);

  return active;
}

gboolean
vinagre_spice_tab_is_mouse_grab (VinagreSpiceTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_TAB (tab), FALSE);

  return tab->priv->mouse_grabbed;
}

/* vim: set ts=8: */
