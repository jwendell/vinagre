/*
 * vinagre-tab.c
 * Abstract base class for all types of tabs: VNC, RDP, etc.
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008,2009 - Jonh Wendell <wendell@bani.com.br>
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
#include <gnome-keyring.h>

#include "vinagre-tab.h"
#include "vinagre-notebook.h"
#include "vinagre-utils.h"
#include "vinagre-prefs.h"
#include "view/autoDrawer.h"
#include "vinagre-plugin.h"
#include "vinagre-plugins-engine.h"

#define VINAGRE_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_TAB, VinagreTabPrivate))

struct _VinagreTabPrivate
{
  GtkWidget         *view;
  GtkWidget         *scroll;
  VinagreConnection *conn;
  VinagreNotebook   *nb;
  VinagreWindow     *window;
  gboolean           save_credentials;
  guint32            keyring_item_id;
  VinagreTabState    state;
  GtkWidget         *layout;
  GtkWidget         *toolbar;
};

G_DEFINE_ABSTRACT_TYPE (VinagreTab, vinagre_tab, GTK_TYPE_VBOX)

/* Signals */
enum
{
  TAB_CONNECTED,
  TAB_DISCONNECTED,
  TAB_INITIALIZED,
  TAB_AUTH_FAILED,
  LAST_SIGNAL
};

/* Properties */
enum
{
  PROP_0,
  PROP_CONN,
  PROP_WINDOW,
  PROP_TOOLTIP
};

static guint signals[LAST_SIGNAL] = { 0 };

static gboolean
vinagre_tab_window_state_cb (GtkWidget           *widget,
			     GdkEventWindowState *event,
			     VinagreTab          *tab)
{
  int view_w, view_h, screen_w, screen_h;
  GdkScreen *screen;
  GtkPolicyType h, v;

  if ((event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) == 0)
    return FALSE;

  vinagre_tab_get_dimensions (tab, &view_w, &view_h);

  screen = gtk_widget_get_screen (GTK_WIDGET (tab));
  screen_w = gdk_screen_get_width (screen);
  screen_h = gdk_screen_get_height (screen);

  if (view_w <= screen_w)
    h = GTK_POLICY_NEVER;
  else
    h = GTK_POLICY_AUTOMATIC;
  
  if (view_h <= screen_h)
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
      case PROP_TOOLTIP:
	g_value_take_string (value, vinagre_tab_get_tooltip (tab));
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
        tab->priv->conn = g_value_dup_object (value);
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
vinagre_tab_dispose (GObject *object)
{
  VinagreTab *tab = VINAGRE_TAB (object);

  if (tab->priv->conn)
    {
      g_signal_handlers_disconnect_by_func (tab->priv->window,
  					    vinagre_tab_window_state_cb,
  					    tab);
      g_object_unref (tab->priv->conn);
      tab->priv->conn = NULL;
    }

  G_OBJECT_CLASS (vinagre_tab_parent_class)->dispose (object);
}

void
default_get_dimensions (VinagreTab *tab, int *w, int *h)
{
  *w = -1;
  *h = -1;
}

const GSList *
default_get_always_sensitive_actions (VinagreTab *tab)
{
  return NULL;
}

const GSList *
default_get_connected_actions (VinagreTab *tab)
{
  return NULL;
}

const GSList *
default_get_initialized_actions (VinagreTab *tab)
{
  return NULL;
}

gchar *
default_get_extra_title (VinagreTab *tab)
{
  return NULL;
}

static void
menu_position (GtkMenu    *menu,
	       gint       *x,
	       gint       *y,
	       gboolean   *push_in,
	       VinagreTab *tab)
{
  int width, height;
  GdkWindow *window;

  window = gtk_widget_get_window (tab->priv->toolbar);

  gdk_window_get_origin (window, x, y);
  gdk_drawable_get_size (window, &width, &height);

  *push_in = TRUE;
  *y += height;
}

static void
open_connection_cb (GtkMenuItem *item,
		    VinagreTab  *tab)
{
  vinagre_window_set_active_tab (tab->priv->window, tab);
}

static void
active_connections_button_clicked  (GtkToolButton *button,
				    VinagreTab    *tab)
{
  GSList            *connections, *l;
  VinagrePlugin     *plugin;
  VinagreConnection *conn;
  GtkWidget         *menu, *item, *image;
  gchar             *str, *label;

  menu = gtk_menu_new ();

  connections = vinagre_notebook_get_tabs (tab->priv->nb);
  for (l = connections; l; l = l->next)
    {
      conn = VINAGRE_TAB (l->data)->priv->conn;
      plugin = vinagre_plugins_engine_get_plugin_by_protocol (vinagre_plugins_engine_get_default (),
							      vinagre_connection_get_protocol (conn));
      item = gtk_image_menu_item_new_with_label ("");
      image = gtk_image_new_from_icon_name (vinagre_plugin_get_icon_name (plugin),
					    GTK_ICON_SIZE_MENU);
      gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);

      label = vinagre_connection_get_best_name (conn);
      if ((l->data == tab) && (GTK_IS_LABEL (GTK_BIN (item)->child)))
	{
	  str = g_strdup_printf ("<b>%s</b>", label);
	  gtk_label_set_use_markup (GTK_LABEL (GTK_BIN (item)->child), TRUE);
	  g_free (label);
	  label = str;
	}
      gtk_menu_item_set_label (GTK_MENU_ITEM (item), label);
      g_free (label);

      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      g_signal_connect (item, "activate", G_CALLBACK (open_connection_cb), l->data);
    }

  gtk_widget_show_all (menu);
  gtk_menu_popup (GTK_MENU (menu),
		  NULL,
		  NULL,
		  (GtkMenuPositionFunc) menu_position,
		  tab,
		  0,
		  gtk_get_current_event_time ());
}

static void
close_button_clicked (GtkToolButton *button,
		      VinagreTab    *tab)
{
  vinagre_notebook_close_tab (tab->priv->nb, tab);
}

static void
fullscreen_button_clicked (GtkToolButton *button,
			   VinagreTab    *tab)
{
  vinagre_window_toggle_fullscreen (tab->priv->window);
}

static void
setup_layout (VinagreTab *tab)
{
  GtkWidget  *button;
  gchar      *str;

  tab->priv->toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (tab->priv->toolbar), FALSE);
  GTK_WIDGET_SET_FLAGS (tab->priv->toolbar, GTK_NO_SHOW_ALL);

  gtk_toolbar_set_style (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOLBAR_BOTH_HORIZ);

  /* Close connection */
  button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_CLOSE));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Close connection"));
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);
  g_signal_connect (button, "clicked", G_CALLBACK (close_button_clicked), tab);

  /* Connection name/menu */
  str = vinagre_connection_get_best_name (tab->priv->conn);
  button = GTK_WIDGET (gtk_tool_button_new (NULL, str));
  g_free (str);

  str = vinagre_connection_get_string_rep (tab->priv->conn, TRUE);
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), str);
  g_free (str);

  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (button), TRUE);
  g_signal_connect (button, "clicked", G_CALLBACK (active_connections_button_clicked), tab);
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);

  /* Leave fullscreen */
  button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_LEAVE_FULLSCREEN));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), _("Leave fullscreen"));
  gtk_widget_show (GTK_WIDGET (button));
  gtk_toolbar_insert (GTK_TOOLBAR (tab->priv->toolbar), GTK_TOOL_ITEM (button), 0);
  g_signal_connect (button, "clicked", G_CALLBACK (fullscreen_button_clicked), tab);

  tab->priv->layout = ViewAutoDrawer_New ();
  ViewAutoDrawer_SetActive (VIEW_AUTODRAWER (tab->priv->layout), FALSE);
  ViewOvBox_SetOver (VIEW_OV_BOX (tab->priv->layout), tab->priv->toolbar);
  ViewOvBox_SetUnder (VIEW_OV_BOX (tab->priv->layout), tab->priv->scroll);
  ViewAutoDrawer_SetOffset (VIEW_AUTODRAWER (tab->priv->layout), -1);
  ViewAutoDrawer_SetFill (VIEW_AUTODRAWER (tab->priv->layout), FALSE);

  gtk_box_pack_end (GTK_BOX(tab), tab->priv->layout, TRUE, TRUE, 0);
  gtk_widget_show_all (GTK_WIDGET (tab));
}

static void
vinagre_tab_constructed (GObject *object)
{
  VinagreTab *tab = VINAGRE_TAB (object);

  if (G_OBJECT_CLASS (vinagre_tab_parent_class)->constructed)
    G_OBJECT_CLASS (vinagre_tab_parent_class)->constructed (object);

  setup_layout (tab);
}

static void 
vinagre_tab_class_init (VinagreTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose  = vinagre_tab_dispose;
  object_class->get_property = vinagre_tab_get_property;
  object_class->set_property = vinagre_tab_set_property;
  object_class->constructed = vinagre_tab_constructed;

  klass->impl_get_tooltip = NULL;
  klass->impl_get_screenshot = NULL;
  klass->impl_get_dimensions = default_get_dimensions;
  klass->impl_get_always_sensitive_actions = default_get_always_sensitive_actions;
  klass->impl_get_connected_actions = default_get_connected_actions;
  klass->impl_get_initialized_actions = default_get_initialized_actions;
  klass->impl_get_extra_title = default_get_extra_title;

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

  g_object_class_install_property (object_class,
				   PROP_TOOLTIP,
				   g_param_spec_string ("tooltip",
							"Tooltip",
							"The tooltip of this tab",
							NULL,
							G_PARAM_READABLE |
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

  signals[TAB_AUTH_FAILED] =
		g_signal_new ("tab-auth-failed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreTabClass, tab_auth_failed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

  g_type_class_add_private (object_class, sizeof (VinagreTabPrivate));
}

void
vinagre_tab_add_recent_used (VinagreTab *tab)
{
  GtkRecentManager *manager;
  GtkRecentData    *data;
  gchar            *uri;

  static gchar *groups[2] = {
		"vinagre",
		NULL
	};

  manager = gtk_recent_manager_get_default ();
  data = g_slice_new (GtkRecentData);

  uri = vinagre_connection_get_string_rep (tab->priv->conn, TRUE);
  data->display_name = vinagre_connection_get_best_name (tab->priv->conn);
  data->description = NULL;
  data->mime_type = g_strdup ("application/x-remote-connection");
  data->app_name = (gchar *) g_get_application_name ();
  data->app_exec = g_strjoin (" ", g_get_prgname (), "%u", NULL);
  data->groups = groups;
  data->is_private = FALSE;

  if (!gtk_recent_manager_add_full (manager, uri, data))
    vinagre_utils_show_error (NULL,
			      _("Error saving recent connection."),
			      GTK_WINDOW (tab->priv->window));

  g_free (uri);
  g_free (data->app_exec);
  g_free (data->mime_type);
  g_free (data->display_name);
  g_slice_free (GtkRecentData, data);
}

static void
vinagre_tab_init (VinagreTab *tab)
{
  tab->priv = VINAGRE_TAB_GET_PRIVATE (tab);
  tab->priv->save_credentials = FALSE;
  tab->priv->keyring_item_id = 0;
  tab->priv->state = VINAGRE_TAB_STATE_INITIALIZING;

  /* Create the scrolled window */
  tab->priv->scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tab->priv->scroll),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (tab->priv->scroll),
				       GTK_SHADOW_NONE);
}

GtkWidget *
vinagre_tab_new (VinagreConnection *conn, VinagreWindow *window)
{
  VinagrePlugin *plugin;
  GtkWidget     *tab;
  const gchar   *protocol = vinagre_connection_get_protocol (conn);

  plugin = g_hash_table_lookup (vinagre_plugin_engine_get_plugins_by_protocol (vinagre_plugins_engine_get_default ()),
				protocol);
  if (!plugin)
    {
      g_warning (_("The protocol %s is not supported."), protocol);
      return NULL;
    }

  return vinagre_plugin_new_tab (plugin, conn, window);
}

gchar *
vinagre_tab_get_tooltip (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return VINAGRE_TAB_GET_CLASS (tab)->impl_get_tooltip (tab);
}

gchar *
vinagre_tab_get_extra_title (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return VINAGRE_TAB_GET_CLASS (tab)->impl_get_extra_title (tab);
}

void
vinagre_tab_get_dimensions (VinagreTab *tab, int *w, int *h)
{
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  VINAGRE_TAB_GET_CLASS (tab)->impl_get_dimensions (tab, w, h);
}

VinagreWindow *
vinagre_tab_get_window (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return tab->priv->window;
}

VinagreConnection *
vinagre_tab_get_conn (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return tab->priv->conn;
}

void
vinagre_tab_add_view (VinagreTab *tab, GtkWidget *view)
{
  GtkWidget *viewport;
  GdkColor color = {0,};

  g_return_if_fail (VINAGRE_IS_TAB (tab));

  tab->priv->view = view;
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (tab->priv->scroll),
					 view);
  viewport = gtk_bin_get_child (GTK_BIN (tab->priv->scroll));
  gtk_viewport_set_shadow_type(GTK_VIEWPORT (viewport), GTK_SHADOW_NONE);
  gtk_widget_modify_bg (viewport, GTK_STATE_NORMAL, &color);
}

GtkWidget *
vinagre_tab_get_view (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return tab->priv->view;
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

VinagreTabState
vinagre_tab_get_state (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), VINAGRE_TAB_STATE_INVALID);

  return tab->priv->state;
}

void
vinagre_tab_set_state (VinagreTab *tab, VinagreTabState state)
{
  tab->priv->state = state;
}

GtkWidget *
vinagre_tab_get_toolbar (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return tab->priv->toolbar;
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
vinagre_tab_find_credentials_in_keyring (VinagreTab *tab, gchar **username, gchar **password)
{
  GnomeKeyringNetworkPasswordData *found_item;
  GnomeKeyringResult               result;
  GList                           *matches;
  
  matches   = NULL;
  *username = NULL;
  *password = NULL;

  result = gnome_keyring_find_network_password_sync (
                vinagre_connection_get_username (tab->priv->conn),            /* user     */
		NULL,                                                         /* domain   */
		vinagre_connection_get_host (tab->priv->conn),                /* server   */
		NULL,                                                         /* object   */
		vinagre_connection_get_protocol (tab->priv->conn),            /* protocol */
		NULL,                                                         /* authtype */
		vinagre_connection_get_port (tab->priv->conn),                /* port     */
		&matches);

  if (result != GNOME_KEYRING_RESULT_OK || matches == NULL || matches->data == NULL)
    return FALSE;

  found_item = (GnomeKeyringNetworkPasswordData *) matches->data;

  *username = g_strdup (found_item->user);
  *password = g_strdup (found_item->password);
  
  tab->priv->keyring_item_id = found_item->item_id;

  gnome_keyring_network_password_list_free (matches);

  return TRUE;
}

void vinagre_tab_set_save_credentials (VinagreTab *tab, gboolean value)
{
  tab->priv->save_credentials = value;
}

void
vinagre_tab_save_credentials_in_keyring (VinagreTab *tab)
{
  GnomeKeyringResult result;

  if (!tab->priv->save_credentials)
    return;

  result = gnome_keyring_set_network_password_sync (
                NULL,                                                        /* default keyring */
                vinagre_connection_get_username (tab->priv->conn),           /* user            */
                NULL,                                                        /* domain          */
                vinagre_connection_get_host (tab->priv->conn),               /* server          */
                NULL,                                                        /* object          */
                vinagre_connection_get_protocol (tab->priv->conn),           /* protocol        */
                NULL,                                                        /* authtype        */
                vinagre_connection_get_port (tab->priv->conn),               /* port            */
                vinagre_connection_get_password (tab->priv->conn),           /* password        */
                &tab->priv->keyring_item_id);

  if (result != GNOME_KEYRING_RESULT_OK)
    vinagre_utils_show_error (_("Error saving the credentials on the keyring."),
			      gnome_keyring_result_to_message (result),
			      GTK_WINDOW (tab->priv->window));

  tab->priv->save_credentials = FALSE;
}

void vinagre_tab_remove_credentials_from_keyring (VinagreTab *tab)
{
  vinagre_connection_set_username (tab->priv->conn, NULL);
  vinagre_connection_set_password (tab->priv->conn, NULL);

  if (tab->priv->keyring_item_id > 0)
    {
      gnome_keyring_item_delete_sync (NULL, tab->priv->keyring_item_id);
      tab->priv->keyring_item_id = 0;
    }
}

void
vinagre_tab_remove_from_notebook (VinagreTab *tab)
{
  vinagre_notebook_close_tab (tab->priv->nb, tab);
}

static void
add_if_writable (GdkPixbufFormat *data, GSList **list)
{
  if (gdk_pixbuf_format_is_writable (data))
    *list = g_slist_prepend (*list, data);
}

static GSList *
get_supported_image_formats (void)
{
  GSList *formats = gdk_pixbuf_get_formats ();
  GSList *writable_formats = NULL;

  g_slist_foreach (formats, (GFunc)add_if_writable, &writable_formats);
  g_slist_free (formats);

  return writable_formats;
}

static void
filter_changed_cb (GObject *object, GParamSpec *pspec, VinagreTab *tab)
{
  GtkFileFilter  *filter;
  gchar          *extension, *filename, *basename, *newbase;
  GtkFileChooser *chooser = GTK_FILE_CHOOSER (object);
  int            i;

  filter = gtk_file_chooser_get_filter (chooser);
  extension = g_object_get_data (G_OBJECT (filter), "extension");

  filename = gtk_file_chooser_get_filename (chooser);
  basename = g_path_get_basename (filename);
  for (i = strlen (basename)-1; i>=0; i--)
    if (basename[i] == '.')
      break;
  basename[i] = '\0';
  newbase = g_strdup_printf ("%s.%s", basename, extension);

  gtk_file_chooser_set_current_name (chooser, newbase);

  g_free (filename);
  g_free (basename);
  g_free (newbase);
}

void
vinagre_tab_take_screenshot (VinagreTab *tab)
{
  GdkPixbuf     *pix;
  GtkWidget     *dialog;
  GString       *suggested_filename;
  gchar         *name;
  GtkFileFilter *filter, *initial_filter;
  GSList        *formats, *l;

  g_return_if_fail (VINAGRE_IS_TAB (tab));

  pix = VINAGRE_TAB_GET_CLASS (tab)->impl_get_screenshot (tab);
  if (!pix)
    {
      vinagre_utils_show_error (NULL,
				_("Could not get a screenshot of the connection."),
				GTK_WINDOW (tab->priv->window));
      return;
    }

  dialog = gtk_file_chooser_dialog_new (_("Save Screenshot"),
				      GTK_WINDOW (tab->priv->window),
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

  name = vinagre_connection_get_best_name (tab->priv->conn);
  suggested_filename = g_string_new (NULL);
  g_string_printf (suggested_filename, _("Screenshot of %s"), name);
  g_string_append (suggested_filename, ".png");
  g_free (name);
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), suggested_filename->str);
  g_string_free (suggested_filename, TRUE);

  formats = get_supported_image_formats ();
  for (l = formats; l; l = l->next)
    {
      GdkPixbufFormat *data = (GdkPixbufFormat *)l->data;
      gchar **exts;
      int i;

      filter = gtk_file_filter_new ();

      name = gdk_pixbuf_format_get_description (data);
      gtk_file_filter_set_name (filter, name);
      g_free (name);

      exts = gdk_pixbuf_format_get_extensions (data);
      g_object_set_data_full (G_OBJECT (filter), "extension", g_strdup (exts[0]), g_free);
      if (strcmp (exts[0], "png") == 0)
	initial_filter = filter;

      for (i = 0; exts[i]; i++)
	{
	  name = g_strdup_printf ("*.%s", exts[i]);
	  gtk_file_filter_add_pattern (filter, name);
	  g_free (name);
	}
      g_strfreev (exts);

      gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
    }
  g_slist_free (formats);

  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), initial_filter);
  g_signal_connect (dialog, "notify::filter", G_CALLBACK (filter_changed_cb), tab);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      GError *error = NULL;
      gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

      filter = gtk_file_chooser_get_filter (GTK_FILE_CHOOSER (dialog));
      name = g_object_get_data (G_OBJECT (filter), "extension");
      if (!name)
	name = "png";

      if (!gdk_pixbuf_save (pix, filename, name, &error, NULL))
	{
	  vinagre_utils_show_error (_("Error saving screenshot"),
				    error->message,
				    GTK_WINDOW (tab->priv->window));
	  g_error_free (error);
	}
      g_free (filename);
  }

  gtk_widget_destroy (dialog);
  g_object_unref (pix);
}

const GSList *
vinagre_tab_get_always_sensitive_actions (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return VINAGRE_TAB_GET_CLASS (tab)->impl_get_always_sensitive_actions (tab);
}

const GSList *
vinagre_tab_get_connected_actions (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return VINAGRE_TAB_GET_CLASS (tab)->impl_get_connected_actions (tab);
}

const GSList *
vinagre_tab_get_initialized_actions (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return VINAGRE_TAB_GET_CLASS (tab)->impl_get_initialized_actions (tab);
}

static void
free_actions (gpointer data, gpointer user_data)
{
  VinagreTabUiAction *action = (VinagreTabUiAction *)data;

  g_strfreev (action->paths);
  g_object_unref (action->action);
  g_free (action);
}

void
vinagre_tab_free_actions (GSList *actions)
{
  g_slist_foreach (actions, (GFunc) free_actions, NULL);
  g_slist_free (actions);
}

const gchar *
vinagre_tab_get_icon_name (VinagreTab *tab)
{
  const gchar   *protocol;
  VinagrePlugin *plugin;

  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  protocol = vinagre_connection_get_protocol (tab->priv->conn);
  plugin = g_hash_table_lookup (vinagre_plugin_engine_get_plugins_by_protocol (vinagre_plugins_engine_get_default ()),
				protocol);
  g_return_val_if_fail (plugin != NULL, NULL);

  return vinagre_plugin_get_icon_name (plugin);
}

/* vim: set ts=8: */
