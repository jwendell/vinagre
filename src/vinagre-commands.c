/*
 * vinagre-help-commands.c
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "vinagre-commands.h"
#include "vinagre-utils.h"
#include "vinagre-connection.h"
#include "vinagre-notebook.h"
#include "vinagre-tab.h"
#include "vinagre-connect.h"
#include "vinagre-favorites.h"
#include "vinagre-fav.h"
#include "vinagre-window-private.h"
#include "vinagre-prefs-manager.h"

/* Machine Menu */
void
vinagre_cmd_machine_connect (GtkAction     *action,
			     VinagreWindow *window)
{
  GtkWidget *tab;
  VinagreConnection *conn;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  conn = vinagre_connect ();
  if (!conn)
    return;

  tab = vinagre_tab_new (conn);
  vinagre_notebook_add_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
			    VINAGRE_TAB (tab),
			    -1);
}

void
vinagre_cmd_machine_close (GtkAction     *action,
			   VinagreWindow *window)
{
  vinagre_window_close_active_tab (window);
}

void
vinagre_cmd_machine_close_all (GtkAction     *action,
			       VinagreWindow *window)
{
  vinagre_window_close_all_tabs (window);
}

/* View Menu */
void
vinagre_cmd_view_show_toolbar	(GtkAction     *action,
				 VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_utils_toggle_widget_visible (window->priv->toolbar);

  vinagre_prefs_manager_set_toolbar_visible (GTK_WIDGET_VISIBLE (window->priv->toolbar));
}

void
vinagre_cmd_view_show_statusbar	(GtkAction     *action,
				 VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_utils_toggle_widget_visible (window->priv->statusbar);

  vinagre_prefs_manager_set_statusbar_visible (GTK_WIDGET_VISIBLE (window->priv->statusbar));
}

void
vinagre_cmd_view_show_fav_panel	(GtkAction     *action,
				 VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_utils_toggle_widget_visible (window->priv->fav_panel);

  vinagre_prefs_manager_set_side_pane_visible (GTK_WIDGET_VISIBLE (window->priv->fav_panel));
}

void
vinagre_cmd_view_fullscreen (GtkAction     *action,
			     VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_window_toggle_fullscreen (window);
}

/* Favorites Menu */
void
vinagre_cmd_open_favorite (VinagreWindow     *window,
			   VinagreConnection *conn)
{
  GtkWidget *tab;
  VinagreConnection *new_conn;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  new_conn = vinagre_connection_clone (conn);
  tab = vinagre_tab_new (new_conn);
  vinagre_notebook_add_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
			    VINAGRE_TAB (tab),
			    -1);
  gtk_widget_show (tab);
}

void
vinagre_cmd_favorites_add (GtkAction     *action,
			   VinagreWindow *window)
{
  VinagreConnection *conn;

  conn = vinagre_tab_get_conn (VINAGRE_TAB (window->priv->active_tab));
  g_return_if_fail (conn != NULL);

  vinagre_favorites_add (conn, window);
  vinagre_tab_set_title (VINAGRE_TAB (window->priv->active_tab),
			 vinagre_connection_best_name (conn));

  vinagre_fav_update_list (VINAGRE_FAV (window->priv->fav_panel));
  vinagre_window_update_favorites_list_menu (window);
}

void
vinagre_cmd_favorites_edit (GtkAction     *action,
			    VinagreWindow *window)
{
  g_return_if_fail (window->priv->fav_conn_selected != NULL);

  if (vinagre_favorites_edit (window->priv->fav_conn_selected, window))
    {
      vinagre_fav_update_list (VINAGRE_FAV (window->priv->fav_panel));
      vinagre_window_update_favorites_list_menu (window);
    }
}

void
vinagre_cmd_favorites_open (GtkAction     *action,
			    VinagreWindow *window)
{
  VinagreConnection *conn = NULL;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  conn = g_object_get_data (G_OBJECT (action), "conn");
  if (!conn)
    conn = window->priv->fav_conn_selected;

  g_return_if_fail (conn != NULL);

  vinagre_cmd_open_favorite (window, conn);
}

void
vinagre_cmd_favorites_del (GtkAction     *action,
			   VinagreWindow *window)
{
  g_return_if_fail (window->priv->fav_conn_selected != NULL);

  if (vinagre_favorites_del (window->priv->fav_conn_selected, window))
    {
      vinagre_fav_update_list (VINAGRE_FAV (window->priv->fav_panel));
      vinagre_window_update_favorites_list_menu (window);
    }
}

/* Help Menu */
void
vinagre_cmd_help_about (GtkAction     *action,
			VinagreWindow *window)
{
  static const gchar * const authors[] = {
	"Jonh Wendell <jwendell@gnome.org>",
	NULL
  };

  static const gchar copyright[] = \
	"Copyright \xc2\xa9 2007 Jonh Wendell";

  static const gchar comments[] = \
	N_("vinagre is a VNC client for the GNOME Desktop");

  gtk_show_about_dialog (GTK_WINDOW (window),
			 "authors", authors,
			 "comments", _(comments),
			 "copyright", copyright,
			 "logo-icon-name", "gnome-remote-desktop",
		         "translator-credits", _("translator-credits"),
			 "version", VERSION,
			 "website", "http://www.gnome.org/~jwendell/vinagre",
			 "name", "vinagre",
			 NULL);
}



