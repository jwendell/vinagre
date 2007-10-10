/*
 * vinagre-window.c
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

#include <time.h>
#include <sys/types.h>
#include <string.h>

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "vinagre-window.h"
#include "vinagre-notebook.h"
#include "vinagre-fav.h"
#include "vinagre-prefs-manager.h"
#include "vinagre-utils.h"
#include "vinagre-bookmarks.h"
#include "vinagre-ui.h"

#include "vinagre-window-private.h"

#define VINAGRE_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					 VINAGRE_TYPE_WINDOW,                    \
					 VinagreWindowPrivate))

G_DEFINE_TYPE(VinagreWindow, vinagre_window, GTK_TYPE_WINDOW)

static void
vinagre_window_finalize (GObject *object)
{
  VinagreWindow *window = VINAGRE_WINDOW (object);

  if (window->priv->fav_conn_selected)
    {
      vinagre_connection_free (window->priv->fav_conn_selected);
      window->priv->fav_conn_selected = NULL;
    }

  if (window->priv->manager)
    {
      g_object_unref (window->priv->manager);
      window->priv->manager = NULL;
    }


  vinagre_bookmarks_finalize ();

  G_OBJECT_CLASS (vinagre_window_parent_class)->finalize (object);
}

static gboolean
vinagre_window_delete_event (GtkWidget   *widget,
			     GdkEventAny *event)
{
  VinagreWindow *window = VINAGRE_WINDOW (widget);

  if (window->priv->signal_notebook > 0)
    g_signal_handler_disconnect (window->priv->notebook,
				 window->priv->signal_notebook);

  vinagre_window_close_all_tabs (window);
  gtk_main_quit ();

  if (GTK_WIDGET_CLASS (vinagre_window_parent_class)->delete_event)
    return GTK_WIDGET_CLASS (vinagre_window_parent_class)->delete_event (widget, event);
  else
    return FALSE;
}

static void
vinagre_window_show_hide_controls (VinagreWindow *window)
{
  if (window->priv->fullscreen)
    {
      window->priv->fav_panel_visible = GTK_WIDGET_VISIBLE (window->priv->fav_panel);
      gtk_widget_hide (window->priv->fav_panel);

      window->priv->toolbar_visible = GTK_WIDGET_VISIBLE (window->priv->toolbar);
      gtk_widget_hide (window->priv->toolbar);

      gtk_widget_hide (window->priv->menubar);

      window->priv->statusbar_visible = GTK_WIDGET_VISIBLE (window->priv->statusbar);
      gtk_widget_hide (window->priv->statusbar);

      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (window->priv->notebook), FALSE);
      gtk_notebook_set_show_border (GTK_NOTEBOOK (window->priv->notebook), FALSE);
    }
  else
    {
      if (window->priv->fav_panel_visible)
        gtk_widget_show_all (window->priv->fav_panel);

      if (window->priv->toolbar_visible)
        gtk_widget_show_all (window->priv->toolbar);

      gtk_widget_show_all (window->priv->menubar);

      if (window->priv->statusbar_visible)
        gtk_widget_show_all (window->priv->statusbar);

      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (window->priv->notebook), TRUE);
      gtk_notebook_set_show_border (GTK_NOTEBOOK (window->priv->notebook), TRUE);
    }
}

static gboolean
vinagre_window_state_event_cb (GtkWidget *widget,
			       GdkEventWindowState *event)
{
  VinagreWindow *window;

  if ((event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) == 0)
    return FALSE;

  window = VINAGRE_WINDOW (widget);

  if (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN)
    window->priv->fullscreen = TRUE;
  else
    window->priv->fullscreen = FALSE;

  window->priv->window_state = event->new_window_state;
  vinagre_prefs_manager_set_window_state (window->priv->window_state);

  vinagre_window_show_hide_controls (window);

  if (GTK_WIDGET_CLASS (vinagre_window_parent_class)->window_state_event)
    return GTK_WIDGET_CLASS (vinagre_window_parent_class)->window_state_event (widget, event);
  else
    return FALSE;
}

static gboolean 
vinagre_window_configure_event (GtkWidget         *widget,
			        GdkEventConfigure *event)
{
  VinagreWindow *window = VINAGRE_WINDOW (widget);

  window->priv->width  = event->width;
  window->priv->height = event->height;

  vinagre_prefs_manager_set_window_size (window->priv->width,
					 window->priv->height);

  return GTK_WIDGET_CLASS (vinagre_window_parent_class)->configure_event (widget, event);
}

/*
static gboolean
vinagre_window_key_press_cb (GtkWidget   *widget,
			     GdkEventKey *event)
{
  VinagreWindow *window = VINAGRE_WINDOW (widget);

  switch (event->keyval)
    {
      case GDK_F11:
	if (window->priv->active_tab)
	  vinagre_window_toggle_fullscreen (window);
	break;
    }

  return GTK_WIDGET_CLASS (vinagre_window_parent_class)->key_press_event (widget, event);
}
*/

static void
vinagre_window_class_init (VinagreWindowClass *klass)
{
  GObjectClass *object_class    = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS (klass);

  object_class->finalize     = vinagre_window_finalize;

  //widget_class->key_press_event    = vinagre_window_key_press_cb;
  widget_class->window_state_event = vinagre_window_state_event_cb;
  widget_class->configure_event    = vinagre_window_configure_event;
  widget_class->delete_event       = vinagre_window_delete_event;

  g_type_class_add_private (object_class, sizeof(VinagreWindowPrivate));
}

static void
menu_item_select_cb (GtkItem       *proxy,
		     VinagreWindow *window)
{
  GtkAction *action;
  char *message;

  action = g_object_get_data (G_OBJECT (proxy),  "gtk-action");
  g_return_if_fail (action != NULL);

  g_object_get (G_OBJECT (action), "tooltip", &message, NULL);
  if (message)
    {
      gtk_statusbar_push (GTK_STATUSBAR (window->priv->statusbar),
			  window->priv->tip_message_cid, message);
      g_free (message);
    }
}

static void
menu_item_deselect_cb (GtkItem       *proxy,
                       VinagreWindow *window)
{
  gtk_statusbar_pop (GTK_STATUSBAR (window->priv->statusbar),
		     window->priv->tip_message_cid);
}

static void
connect_proxy_cb (GtkUIManager  *manager,
                  GtkAction     *action,
                  GtkWidget     *proxy,
                  VinagreWindow *window)
{
  if (GTK_IS_MENU_ITEM (proxy))
    {
       g_signal_connect (proxy, "select",
			 G_CALLBACK (menu_item_select_cb), window);
       g_signal_connect (proxy, "deselect",
			 G_CALLBACK (menu_item_deselect_cb), window);
    }
}

static void
disconnect_proxy_cb (GtkUIManager  *manager,
                     GtkAction     *action,
                     GtkWidget     *proxy,
                     VinagreWindow *window)
{
  if (GTK_IS_MENU_ITEM (proxy))
    {
      g_signal_handlers_disconnect_by_func
		(proxy, G_CALLBACK (menu_item_select_cb), window);
      g_signal_handlers_disconnect_by_func
		(proxy, G_CALLBACK (menu_item_deselect_cb), window);
    }
}

static void
activate_recent_cb (GtkRecentChooser *action, VinagreWindow *window)
{
  VinagreConnection *conn;

  conn = vinagre_connection_new_from_string (gtk_recent_chooser_get_current_uri (action));
  vinagre_cmd_open_bookmark (window, conn);

  vinagre_connection_free (conn);
}

static void update_recent_connections (VinagreWindow *window)
{
  VinagreWindowPrivate *p = window->priv;

  g_return_if_fail (p->recent_action_group != NULL);

  if (p->recents_menu_ui_id != 0)
    gtk_ui_manager_remove_ui (p->manager, p->recents_menu_ui_id);

  p->recents_menu_ui_id = gtk_ui_manager_new_merge_id (p->manager);

  gtk_ui_manager_add_ui (p->manager,
			 p->recents_menu_ui_id,
			 "/MenuBar/MachineMenu/FileRecentsPlaceholder",
			 "recent_connections",
			 "recent_connections",
			 GTK_UI_MANAGER_MENUITEM,
			 FALSE);
}

static void
recent_manager_changed (GtkRecentManager *manager,
			VinagreWindow     *window)
{
  update_recent_connections (window);
}

static void
create_menu_bar_and_toolbar (VinagreWindow *window, 
			     GtkWidget     *main_box)
{
  GtkActionGroup *action_group;
  GtkUIManager *manager;
  GError *error = NULL;
  GtkRecentManager *recent_manager;
  GtkRecentFilter *filter;

  manager = gtk_ui_manager_new ();
  window->priv->manager = manager;

  /* show tooltips in the statusbar */
  g_signal_connect (manager,
		    "connect-proxy",
		    G_CALLBACK (connect_proxy_cb),
		    window);
  g_signal_connect (manager,
		   "disconnect-proxy",
		    G_CALLBACK (disconnect_proxy_cb),
		    window);

  gtk_window_add_accel_group (GTK_WINDOW (window),
			      gtk_ui_manager_get_accel_group (manager));

  action_group = gtk_action_group_new ("VinagreWindowAlwaysSensitiveActions");
  gtk_action_group_set_translation_domain (action_group, NULL);
  gtk_action_group_add_actions (action_group,
				vinagre_always_sensitive_menu_entries,
				G_N_ELEMENTS (vinagre_always_sensitive_menu_entries),
				window);
  
  gtk_action_group_add_toggle_actions (action_group,
				       vinagre_always_sensitive_toggle_menu_entries,
				       G_N_ELEMENTS (vinagre_always_sensitive_toggle_menu_entries),
				       window);

  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);
  window->priv->always_sensitive_action_group = action_group;

  action_group = gtk_action_group_new ("VinagreWindowActions");
  gtk_action_group_set_translation_domain (action_group, NULL);
  gtk_action_group_add_actions (action_group,
				vinagre_menu_entries,
				G_N_ELEMENTS (vinagre_menu_entries),
				window);

  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);
  window->priv->action_group = action_group;

  /* now load the UI definition */
  gtk_ui_manager_add_ui_from_file (manager, vinagre_utils_get_ui_xml_filename (), &error);
  if (error != NULL)
    {
      g_warning (_("Could not merge vinagre-ui.xml: %s"), error->message);
      g_error_free (error);
    }

  /* Bookmarks */
  action_group = gtk_action_group_new ("BookmarksActions");
  gtk_action_group_set_translation_domain (action_group, NULL);
  window->priv->bookmarks_list_action_group = action_group;
  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);

  window->priv->menubar = gtk_ui_manager_get_widget (manager, "/MenuBar");
  gtk_box_pack_start (GTK_BOX (main_box), 
		      window->priv->menubar, 
		      FALSE, 
		      FALSE, 
		      0);

  window->priv->toolbar = gtk_ui_manager_get_widget (manager, "/ToolBar");
  gtk_widget_hide (window->priv->toolbar);
  gtk_box_pack_start (GTK_BOX (main_box),
		      window->priv->toolbar,
		      FALSE,
		      FALSE,
		      0);

  /* Recent connections */
  window->priv->recent_action = gtk_recent_action_new ("recent_connections",
						     _("_Recent connections"),
						       NULL, NULL);
  g_signal_connect (window->priv->recent_action,
		    "item-activated",
		    G_CALLBACK (activate_recent_cb),
		    window);

  action_group = gtk_action_group_new ("VinagreRecentConnectionsActions");
  gtk_action_group_set_translation_domain (action_group, NULL);

  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);
  window->priv->recent_action_group = action_group;
  gtk_action_group_add_action (window->priv->recent_action_group,
			       window->priv->recent_action);

  filter = gtk_recent_filter_new ();
  gtk_recent_filter_add_group (filter, "vinagre");
  gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (window->priv->recent_action),
				 filter);

  update_recent_connections (window);

  recent_manager = gtk_recent_manager_get_default ();
  g_signal_connect (recent_manager,
		    "changed",
		    G_CALLBACK (recent_manager_changed),
		    window);

}

static void
set_machine_menu_sensitivity (VinagreWindow *window)
{
  gboolean active;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  active = gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->priv->notebook)) > 0;
  gtk_action_group_set_sensitive (window->priv->action_group, active);
}

static void
fav_panel_size_allocate (GtkWidget     *widget,
			 GtkAllocation *allocation,
			 VinagreWindow *window)
{
  window->priv->side_panel_size = allocation->width;
  if (window->priv->side_panel_size > 0)
    vinagre_prefs_manager_set_side_panel_size (window->priv->side_panel_size);
}

static void
fav_panel_activated (VinagreFav        *fav,
		     VinagreConnection *conn,
		     VinagreWindow     *window)
{
  g_return_if_fail (VINAGRE_IS_FAV (fav));
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_cmd_open_bookmark (window, conn);
}

static void
fav_panel_selected (VinagreFav        *fav,
		    VinagreConnection *conn,
		    VinagreWindow     *window)
{
  GtkAction *action1, *action2;

  action1 = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					 "BookmarksDel");
  action2 = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					 "BookmarksEdit");

  if (window->priv->fav_conn_selected)
    {
      vinagre_connection_free (window->priv->fav_conn_selected);
      window->priv->fav_conn_selected = NULL;
    }

  if (conn)
    {
      window->priv->fav_conn_selected = vinagre_connection_clone (conn);
      gtk_action_set_sensitive (action1, TRUE);
      gtk_action_set_sensitive (action2, TRUE);
      /* TODO: Change the menuitem label */
    }
  else
    {
      /* TODO: Change the menuitem label */
      gtk_action_set_sensitive (action1, FALSE);
      gtk_action_set_sensitive (action2, FALSE);
    }
}

void
vinagre_window_update_bookmarks_list_menu (VinagreWindow *window)
{
  VinagreWindowPrivate *p = window->priv;
  GList                *actions, *l, *favs;
  gint                  n, i;
  guint                 id;
  VinagreConnection    *conn;
  gchar                *action_name, *action_label;
  GtkAction            *action;
  gchar                *name;

  g_return_if_fail (p->bookmarks_list_action_group != NULL);

  if (p->bookmarks_list_menu_ui_id != 0)
    gtk_ui_manager_remove_ui (p->manager, p->bookmarks_list_menu_ui_id);

  actions = gtk_action_group_list_actions (p->bookmarks_list_action_group);
  for (l = actions; l != NULL; l = l->next)
    {
      g_signal_handlers_disconnect_by_func (GTK_ACTION (l->data),
					    G_CALLBACK (vinagre_cmd_bookmarks_open),
					    window);
      gtk_action_group_remove_action (p->bookmarks_list_action_group,
				      GTK_ACTION (l->data));
    }
    g_list_free (actions);

  /* Get the bookmarks from file */
  favs = vinagre_bookmarks_get_all ();
  n = g_list_length (favs);
  i = 0;

  id = (n > 0) ? gtk_ui_manager_new_merge_id (p->manager) : 0;

  while (favs)
    {
      conn = (VinagreConnection *) favs->data;
      name = vinagre_connection_best_name (conn);

      action_name = g_strdup_printf ("Fav_%d", i);
      action_label = vinagre_utils_escape_underscores (
		     name,
		     -1);
      action = gtk_action_new (action_name,
			       action_label,
			       NULL,
			       NULL);
      g_object_set_data_full (G_OBJECT (action),
			      "conn",
			      conn,
			      (GDestroyNotify) vinagre_connection_free);
      gtk_action_group_add_action (p->bookmarks_list_action_group,
				   GTK_ACTION (action));

      g_signal_connect (action,
			"activate",
			G_CALLBACK (vinagre_cmd_bookmarks_open),
			window);

      gtk_ui_manager_add_ui (p->manager,
			     id,
			     "/MenuBar/BookmarksMenu/BookmarksList",
			     action_name, action_name,
			     GTK_UI_MANAGER_MENUITEM,
			     FALSE);

      g_object_unref (action);
      g_free (action_name);
      g_free (action_label);
      g_free (name);

      favs = favs->next;
      i++;
    }

  g_list_free (favs);

  p->bookmarks_list_menu_ui_id = id;
}

static void
create_side_panel (VinagreWindow *window)
{
  window->priv->fav_panel = vinagre_fav_new (window);

  gtk_paned_pack1 (GTK_PANED (window->priv->hpaned), 
		   window->priv->fav_panel, 
		   FALSE, 
		   FALSE);

  window->priv->side_panel_size = vinagre_prefs_manager_get_side_panel_size ();
  gtk_paned_set_position (GTK_PANED (window->priv->hpaned), window->priv->side_panel_size);

  g_signal_connect (window->priv->fav_panel,
		    "size-allocate",
		    G_CALLBACK (fav_panel_size_allocate),
		    window);
  g_signal_connect (window->priv->fav_panel,
		    "fav-activated",
		    G_CALLBACK (fav_panel_activated),
		    window);
  g_signal_connect (window->priv->fav_panel,
		    "fav-selected",
		    G_CALLBACK (fav_panel_selected),
		    window);
}

static void
init_widgets_visibility (VinagreWindow *window)
{
  GdkWindowState state;
  gint w, h;
  GtkAction *action;
  gboolean visible;

  /* Remove and Edit bookmarks starts disabled */
  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"BookmarksDel");
  gtk_action_set_sensitive (action, FALSE);

  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"BookmarksEdit");
  gtk_action_set_sensitive (action, FALSE);

  /* TODO: Implement these commands */
  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"HelpContents");
  gtk_action_set_sensitive (action, FALSE);

  /* fav panel visibility */
  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"ViewBookmarks");
  visible = vinagre_prefs_manager_get_side_pane_visible ();
  if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) != visible)
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);

  /* toolbar visibility */
  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"ViewToolbar");
  visible = vinagre_prefs_manager_get_toolbar_visible ();
  if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) != visible)
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);

  /* statusbar visibility */
  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"ViewStatusbar");
  visible = vinagre_prefs_manager_get_side_pane_visible ();
  if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) != visible)
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);

  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
				vinagre_prefs_manager_get_statusbar_visible ());

//  if (vinagre_prefs_manager_get_side_pane_visible ())
//    gtk_widget_show_all (window->priv->fav_panel);
//  else
//    {
//      gtk_widget_hide (window->priv->fav_panel);
//printf ("hiding... was visible: %d\n", GTK_WIDGET_VISIBLE(window->priv->fav_panel));

//      gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);
//    }

  /* toolbar visibility */
//  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
//					"ViewToolbar");
//  if (vinagre_prefs_manager_get_toolbar_visible ())
//    gtk_widget_show_all (window->priv->toolbar);
//  else
//    {
//      gtk_widget_hide (window->priv->toolbar);
//      gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);
//   }

  /* toolbar visibility */
//  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
//					"ViewStatusbar");
//  if (vinagre_prefs_manager_get_statusbar_visible ())
//    gtk_widget_show_all (window->priv->statusbar);
//  else
//    {
 //     gtk_widget_hide (window->priv->statusbar);
//      gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);
 //   }

  state = vinagre_prefs_manager_get_window_state ();
  vinagre_prefs_manager_get_window_size (&w, &h);

  gtk_window_set_default_size (GTK_WINDOW (window), w, h);

  if ((state & GDK_WINDOW_STATE_MAXIMIZED) != 0)
    gtk_window_maximize (GTK_WINDOW (window));
  else
    gtk_window_unmaximize (GTK_WINDOW (window));
}

static void
create_statusbar (VinagreWindow *window, 
		  GtkWidget   *main_box)
{
  window->priv->statusbar = gtk_statusbar_new ();

  window->priv->generic_message_cid = gtk_statusbar_get_context_id
	(GTK_STATUSBAR (window->priv->statusbar), "generic_message");
  window->priv->tip_message_cid = gtk_statusbar_get_context_id
	(GTK_STATUSBAR (window->priv->statusbar), "tip_message");

  gtk_box_pack_end (GTK_BOX (main_box),
		    window->priv->statusbar,
		    FALSE, 
		    TRUE, 
		    0);
}

void
vinagre_window_set_title (VinagreWindow *window)
{
  gchar *title, *name;

  if (window->priv->active_tab == NULL)
    {
      gtk_window_set_title (GTK_WINDOW (window), "vinagre");
      return;
    }

  name = vinagre_connection_best_name (vinagre_tab_get_conn (VINAGRE_TAB (window->priv->active_tab)));
  title = g_strdup_printf ("%s - %s",
			   name,
			   "vinagre");
  gtk_window_set_title (GTK_WINDOW (window), title);
  g_free (title);
  g_free (name);
}

static void
vinagre_window_page_removed (GtkNotebook   *notebook,
			     GtkWidget     *child,
			     guint         page_num,
			     VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  window->priv->active_tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->priv->notebook), gtk_notebook_get_current_page (GTK_NOTEBOOK (window->priv->notebook)));

  if (!window->priv->active_tab && window->priv->fullscreen)
    vinagre_window_toggle_fullscreen (window);

  vinagre_window_set_title (window);
  set_machine_menu_sensitivity (window);
}

static void
vinagre_window_page_added (GtkNotebook  *notebook,
			   GtkWidget     *child,
			   guint         page_num,
			   VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  window->priv->active_tab = child;

  vinagre_window_set_title (window);
  set_machine_menu_sensitivity (window);
}

static void 
vinagre_window_switch_page (GtkNotebook     *notebook, 
			    GtkNotebookPage *pg,
			    gint            page_num, 
			    VinagreWindow   *window)
{
  GtkWidget *tab;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  tab = gtk_notebook_get_nth_page (notebook, page_num);
  if (tab == window->priv->active_tab)
    return;

  window->priv->active_tab = tab;

  vinagre_window_set_title (window);
}

static void
create_notebook (VinagreWindow *window)
{
  window->priv->notebook = vinagre_notebook_new (window);

  gtk_paned_pack2 (GTK_PANED (window->priv->hpaned), 
  		   window->priv->notebook,
  		   TRUE, 
  		   FALSE);

  window->priv->signal_notebook =
   g_signal_connect_after (window->priv->notebook,
		          "page-removed",
		          G_CALLBACK (vinagre_window_page_removed),
		          window);

  g_signal_connect_after (window->priv->notebook,
		          "page-added",
		          G_CALLBACK (vinagre_window_page_added),
		          window);

  g_signal_connect (window->priv->notebook,
		    "switch_page",
		    G_CALLBACK (vinagre_window_switch_page),
		    window);

  gtk_widget_show (window->priv->notebook);
}

static void
vinagre_window_clipboard_cb (GtkClipboard *cb, GdkEvent *event, VinagreWindow *window)
{
  gchar *text;

  text = gtk_clipboard_wait_for_text (cb);
  if (text) {
    if (window->priv->active_tab)
      vinagre_tab_paste_text (VINAGRE_TAB (window->priv->active_tab), text);
    g_free (text);
  }

}

static void
vinagre_window_init_clipboard (VinagreWindow *window)
{
  GtkClipboard *cb;

  cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  g_signal_connect (cb, "owner-change", G_CALLBACK (vinagre_window_clipboard_cb), window);
}

static void
vinagre_window_init (VinagreWindow *window)
{
  GtkWidget *main_box;

  gtk_window_set_default_icon_name ("vinagre");

  window->priv = VINAGRE_WINDOW_GET_PRIVATE (window);
  window->priv->active_tab = NULL;
  window->priv->fav_conn_selected = NULL;
  window->priv->fullscreen = FALSE;
  window->priv->signal_notebook = 0;

  vinagre_bookmarks_init ();

  main_box = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), main_box);
  gtk_widget_show (main_box);

  /* Add menu bar and toolbar bar  */
  create_menu_bar_and_toolbar (window, main_box);

  /* Add status bar */
  create_statusbar (window, main_box);

  /* Add the main area */
  window->priv->hpaned = gtk_hpaned_new ();
  gtk_box_pack_start (GTK_BOX (main_box), 
  		      window->priv->hpaned, 
  		      TRUE, 
  		      TRUE, 
  		      0);

  /* setup notebook area */
  create_notebook (window);

  /* side panel */
  create_side_panel (window);

  gtk_widget_show (window->priv->hpaned);

  gtk_widget_grab_focus (window->priv->hpaned);

  init_widgets_visibility (window);
  set_machine_menu_sensitivity (window);

  vinagre_window_update_bookmarks_list_menu (window);
  vinagre_window_init_clipboard (window);
}

GtkWidget *
vinagre_window_get_notebook (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->notebook;
}

void
vinagre_window_close_tab (VinagreWindow *window,
			  VinagreTab    *tab)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));
  g_return_if_fail (VINAGRE_IS_TAB (tab));
	
  vinagre_notebook_remove_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
			       tab);
}

void
vinagre_window_close_all_tabs (VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_notebook_remove_all_tabs (VINAGRE_NOTEBOOK (window->priv->notebook));
}


void
vinagre_window_set_active_tab (VinagreWindow *window,
			       VinagreTab    *tab)
{
  gint page_num;
	
  g_return_if_fail (VINAGRE_IS_WINDOW (window));
  g_return_if_fail (VINAGRE_IS_TAB (tab));
	
  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (window->priv->notebook),
				    GTK_WIDGET (tab));
  g_return_if_fail (page_num != -1);
	
  gtk_notebook_set_current_page (GTK_NOTEBOOK (window->priv->notebook),
				 page_num);
}

VinagreWindow *
vinagre_window_new ()
{
  return g_object_new (VINAGRE_TYPE_WINDOW,
		       "type",      GTK_WINDOW_TOPLEVEL,
		       "title",     "Vinagre",
		       NULL); 
}

gboolean
vinagre_window_is_fullscreen (VinagreWindow *window)
{
  return window->priv->fullscreen;
}

void
vinagre_window_toggle_fullscreen (VinagreWindow *window)
{
  if (window->priv->fullscreen)
    gtk_window_unfullscreen (GTK_WINDOW (window));
  else
    gtk_window_fullscreen (GTK_WINDOW (window));
}

GtkWidget *
vinagre_window_get_statusbar (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->statusbar;
}

GtkWidget *
vinagre_window_get_toolbar (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->toolbar;
}

GtkWidget *
vinagre_window_get_fav_panel (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->fav_panel;
}

void
vinagre_window_close_active_tab	(VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_window_close_tab (window,
			    VINAGRE_TAB (window->priv->active_tab));
}

VinagreTab *
vinagre_window_get_active_tab	(VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return VINAGRE_TAB (window->priv->active_tab);
}

GtkUIManager *
vinagre_window_get_ui_manager (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->manager;
}

