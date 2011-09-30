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
#include "vinagre-connection.h"
#include "vinagre-notebook.h"
#include "vinagre-tab.h"
#include "vinagre-connect.h"
#include "vinagre-bookmarks.h"
#include "vinagre-bookmarks-ui.h"
#include "vinagre-window-private.h"
#include "vinagre-prefs.h"
#include "vinagre-cache-prefs.h"
#include "vinagre-plugins-engine.h"
#include "vinagre-reverse-vnc-listener-dialog.h"
#include "vinagre-vala.h"

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

/* Remote Menu */
void
vinagre_cmd_remote_connect (GtkAction     *action,
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

void
vinagre_cmd_remote_open (GtkAction     *action,
			  VinagreWindow *window)
{
  GtkWidget         *tab;
  VinagreConnection *conn;
  GtkWidget         *dialog;
  GtkFileFilter     *filter;
  GSList            *files, *l;
  gchar             *uri;
  gchar             *error = NULL;
  GSList            *errors = NULL;
  gint              i = 0;
  GHashTable        *protocols;
  GHashTableIter    iter;
  VinagreProtocol   *protocol;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  dialog = gtk_file_chooser_dialog_new (_("Choose the file"),
					GTK_WINDOW (window),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), FALSE);
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);

  protocols = vinagre_plugins_engine_get_plugins_by_protocol (vinagre_plugins_engine_get_default ());
  g_hash_table_iter_init (&iter, protocols);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *)&protocol))
    {
      filter = vinagre_protocol_get_file_filter (protocol);
      if (filter)
	{
	  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	  i++;
	}
    }

  if (i == 0)
    {
      vinagre_utils_show_error_dialog (_("There are no supported files"),
				_("None of the active plugins support this action. Activate some plugins and try again."),
				GTK_WINDOW (window));
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
      g_slist_free_full (errors, g_free);
    }

finalize:
  gtk_widget_destroy (dialog);
}

void
vinagre_cmd_remote_disconnect (GtkAction     *action,
			   VinagreWindow *window)
{
  vinagre_window_close_active_tab (window);
}

void
vinagre_cmd_remote_take_screenshot (GtkAction     *action,
				     VinagreWindow *window)
{
  vinagre_tab_take_screenshot (vinagre_window_get_active_tab (window));
}

void
vinagre_cmd_remote_disconnect_all (GtkAction     *action,
			       VinagreWindow *window)
{
  vinagre_window_close_all_tabs (window);
}

void
vinagre_cmd_remote_vnc_listener (GtkAction *action,
                                 VinagreWindow *window)
{
    vinagre_reverse_vnc_listener_dialog_show (GTK_WINDOW (window));
}

void
vinagre_cmd_remote_quit (GtkAction     *action,
			  VinagreWindow *window)
{
  gtk_widget_destroy (GTK_WIDGET (window));
}

/* View Menu */
void
vinagre_cmd_view_show_toolbar	(GtkAction     *action,
				 VinagreWindow *window)
{
  gboolean active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_utils_set_widget_visible (window->priv->toolbar, active);

  vinagre_cache_prefs_set_boolean ("window",
				   "toolbar-visible",
				   gtk_widget_get_visible (window->priv->toolbar));
}

void
vinagre_cmd_view_show_statusbar	(GtkAction     *action,
				 VinagreWindow *window)
{
  gboolean active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_utils_set_widget_visible (window->priv->statusbar, active);

  vinagre_cache_prefs_set_boolean ("window",
				   "statusbar-visible",
				   gtk_widget_get_visible (window->priv->statusbar));
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

/* Help Menu */

void
vinagre_cmd_help_contents (GtkAction     *action,
			   VinagreWindow *window)
{
  vinagre_utils_show_help (GTK_WINDOW (window), NULL);
}

void
vinagre_cmd_help_about (GtkAction     *action,
			VinagreWindow *window)
{
  vinagre_utils_show_help_about (GTK_WINDOW (window));
}
