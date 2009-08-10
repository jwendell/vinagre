/*
 * vinagre-commands.c
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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#include "vinagre-commands.h"
#include "vinagre-utils.h"
#include "vinagre-connection.h"
#include "vinagre-notebook.h"
#include "vinagre-tab.h"
#include "vinagre-connect.h"
#include "vinagre-bookmarks.h"
#include "vinagre-bookmarks-ui.h"
#include "vinagre-fav.h"
#include "vinagre-window-private.h"
#include "vinagre-prefs.h"
#include "vinagre-plugin.h"
#include "vinagre-plugin-info.h"
#include "vinagre-plugin-info-priv.h"
#include "vinagre-plugins-engine.h"

void
vinagre_cmd_direct_connect (VinagreConnection *conn,
			    VinagreWindow     *window)
{
  VinagreTab *tab;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  tab = vinagre_window_conn_exists (window, conn);
  if (tab)
    {
      vinagre_window_set_active_tab (window, tab);
    }
  else
    {
      tab = VINAGRE_TAB (vinagre_tab_new (conn, window));
      vinagre_notebook_add_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
				VINAGRE_TAB (tab),
				-1);
    }
}

/* Machine Menu */
void
vinagre_cmd_machine_connect (GtkAction     *action,
			     VinagreWindow *window)
{
  VinagreTab *tab;
  VinagreConnection *conn;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  conn = vinagre_connect (window);
  if (!conn)
    return;

  tab = vinagre_window_conn_exists (window, conn);
  if (tab)
    {
      vinagre_window_set_active_tab (window, tab);
    }
  else
    {
      tab = VINAGRE_TAB (vinagre_tab_new (conn, window));
      vinagre_notebook_add_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
				VINAGRE_TAB (tab),
				-1);
    }

  g_object_unref (conn);
}

static void
vinagre_cmd_free_string_list (gpointer str, gpointer user_data)
{
  g_free (str);
}

void
vinagre_cmd_machine_open (GtkAction     *action,
			  VinagreWindow *window)
{
  GtkWidget         *tab;
  VinagreConnection *conn;
  GtkWidget         *dialog;
  GtkFileFilter     *filter;
  GSList            *files, *l;
  gchar             *uri;
  gchar             *error = NULL;
  GSList            *plugins, *errors = NULL;
  gint              i;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  dialog = gtk_file_chooser_dialog_new (_("Choose the file"),
					GTK_WINDOW (window),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), FALSE);
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);

  plugins = (GSList *) vinagre_plugins_engine_get_plugin_list (vinagre_plugins_engine_get_default ());
  i = 0;
  for (; plugins; plugins = plugins->next)
    {
      VinagrePluginInfo *info = VINAGRE_PLUGIN_INFO (plugins->data);

      if (!vinagre_plugin_info_is_active (info))
	continue;

      filter = vinagre_plugin_get_file_filter (info->plugin);
      if (filter)
	{
	  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	  i++;
	}
    }
  if (i == 0)
    {
      vinagre_utils_show_error (_("There are none supported files"), _("None of the active plugins offer a supported file to be open. Activate some plugins and try again."), GTK_WINDOW (window));
      goto finalize;
    }

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      files = gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));
      for (l = files; l; l = l->next)
	{
	  uri = (gchar *)l->data;
	  conn = vinagre_connection_new_from_file (uri, &error, FALSE);

	  if (conn)
	    {
	      tab = vinagre_tab_new (conn, window);
	      vinagre_notebook_add_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
					VINAGRE_TAB (tab),
					-1);
	      g_object_unref (conn);
	    }
	  else
	    {
	      errors = g_slist_append (errors, g_strdup_printf ("<i>%s</i>: %s", uri, error?error:_("Unknown error")));
	      g_free (error);
	    }

	  g_free (uri);
	}
      g_slist_free (files);
    }

  if (errors)
    {
      vinagre_utils_show_many_errors (ngettext ("The following file could not be opened:",
						 "The following files could not be opened:",
						 g_slist_length (errors)),
				      errors,
				      GTK_WINDOW (window));
      g_slist_foreach (errors, vinagre_cmd_free_string_list, NULL);
      g_slist_free (errors);
    }

finalize:
  gtk_widget_destroy (dialog);
}

void
vinagre_cmd_machine_close (GtkAction     *action,
			   VinagreWindow *window)
{
  vinagre_window_close_active_tab (window);
}

void
vinagre_cmd_machine_take_screenshot (GtkAction     *action,
				     VinagreWindow *window)
{
  vinagre_tab_take_screenshot (vinagre_window_get_active_tab (window));
}

void
vinagre_cmd_machine_close_all (GtkAction     *action,
			       VinagreWindow *window)
{
  vinagre_window_close_all_tabs (window);
}

void
vinagre_cmd_machine_quit (GtkAction     *action,
			  VinagreWindow *window)
{
  gtk_widget_destroy (GTK_WIDGET (window));
}

/* Edit Menu */
void
vinagre_cmd_edit_preferences (GtkAction     *action,
			      VinagreWindow *window)
{
  vinagre_prefs_dialog_show (window);
}

/*
FIXME: Study this dialog for next release
void
vinagre_cmd_edit_plugins (GtkAction     *action,
                          VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_plugin_dialog_show (GTK_WINDOW (window));
}
*/

/* View Menu */
void
vinagre_cmd_view_show_toolbar	(GtkAction     *action,
				 VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_utils_toggle_widget_visible (window->priv->toolbar);

  g_object_set (vinagre_prefs_get_default (),
		"toolbar-visible", GTK_WIDGET_VISIBLE (window->priv->toolbar),
		NULL);
}

void
vinagre_cmd_view_show_statusbar	(GtkAction     *action,
				 VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_utils_toggle_widget_visible (window->priv->statusbar);

  g_object_set (vinagre_prefs_get_default (),
		"statusbar-visible", GTK_WIDGET_VISIBLE (window->priv->statusbar),
		NULL);
}

void
vinagre_cmd_view_show_fav_panel	(GtkAction     *action,
				 VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_utils_toggle_widget_visible (window->priv->fav_panel);

  g_object_set (vinagre_prefs_get_default (),
		"side-panel-visible", GTK_WIDGET_VISIBLE (window->priv->fav_panel),
		NULL);
}

void
vinagre_cmd_view_fullscreen (GtkAction     *action,
			     VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_window_toggle_fullscreen (window);
}

/* Bookmarks Menu */
void
vinagre_cmd_open_bookmark (VinagreWindow     *window,
			   VinagreConnection *conn)
{
  VinagreTab *tab;
  VinagreConnection *new_conn;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  tab = vinagre_window_conn_exists (window, conn);
  if (tab)
    {
      vinagre_window_set_active_tab (window, tab);
    }
  else
    {
      tab = VINAGRE_TAB (vinagre_tab_new (conn, window));
      vinagre_notebook_add_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
				VINAGRE_TAB (tab),
				-1);
    }
}

void
vinagre_cmd_bookmarks_add (GtkAction     *action,
			   VinagreWindow *window)
{
  VinagreTab        *tab;
  VinagreConnection *conn;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  tab = vinagre_window_get_active_tab (window);
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  conn = vinagre_tab_get_conn (tab);
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  vinagre_bookmarks_add (vinagre_bookmarks_get_default (),
                         conn,
                         GTK_WINDOW (window));

  if (vinagre_window_get_active_tab (window) == tab)
    {
      gchar *name = vinagre_connection_get_best_name (conn);
      vinagre_tab_set_title (tab, name);
      g_free (name);
    }
}

void
vinagre_cmd_bookmarks_new_folder (GtkAction     *action,
				  VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_bookmarks_new_folder (vinagre_bookmarks_get_default (),
				GTK_WINDOW (window));
}

void
vinagre_cmd_bookmarks_edit (GtkAction     *action,
			    VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));
  g_return_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (window->priv->fav_entry_selected));

  vinagre_bookmarks_edit (vinagre_bookmarks_get_default (),
                          window->priv->fav_entry_selected,
                          GTK_WINDOW (window));
}

void
vinagre_cmd_bookmarks_del (GtkAction     *action,
			   VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));
  g_return_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (window->priv->fav_entry_selected));

  vinagre_bookmarks_del (vinagre_bookmarks_get_default (),
                         window->priv->fav_entry_selected,
                         GTK_WINDOW (window));
}

void
vinagre_cmd_bookmarks_open (GtkAction     *action,
			    VinagreWindow *window)
{
  VinagreConnection *conn;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  conn = g_object_get_data (G_OBJECT (action), "conn");
  if (!conn)
    conn = vinagre_bookmarks_entry_get_conn (window->priv->fav_entry_selected);

  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  vinagre_cmd_open_bookmark (window, conn);
}

/* Help Menu */

void
vinagre_cmd_help_contents (GtkAction     *action,
			   VinagreWindow *window)
{
  vinagre_utils_help_contents (GTK_WINDOW (window));
}

void
vinagre_cmd_help_about (GtkAction     *action,
			VinagreWindow *window)
{
  vinagre_utils_help_about (GTK_WINDOW (window));
}

/* vim: set ts=8: */
