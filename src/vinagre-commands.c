/*
 * vinagre-commands.c
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
#include "vinagre-fav.h"
#include "vinagre-window-private.h"
#include "vinagre-prefs.h"

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
  gchar             *error;
  GSList            *errors = NULL;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  dialog = gtk_file_chooser_dialog_new (_("Choose the file"),
					GTK_WINDOW (window),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), FALSE);
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Supported formats"));
  gtk_file_filter_add_pattern (filter, "*.vnc");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      files = gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));
      for (l = files; l; l = l->next)
        {
	  uri = (gchar *)l->data;
	  conn = vinagre_connection_new_from_file (uri, &error);

	  if (conn)
	    {
	      tab = vinagre_tab_new (conn, window);
	      vinagre_notebook_add_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
				        VINAGRE_TAB (tab),
				        -1);
	    }
	  else
	    {
	      errors = g_slist_append (errors, g_strdup (uri));
	      if (error)
	        g_free (error);
	    }

	  g_free (uri);
	}
      g_slist_free (files);
    }

  if (errors)
    vinagre_utils_show_many_errors (ngettext ("The following file could not be opened:",
					      "The following files could not be opened:",
					      g_slist_length (errors)),
				    errors,
				    GTK_WINDOW (window));

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
vinagre_cmd_machine_send_ctrlaltdel (GtkAction     *action,
				     VinagreWindow *window)
{
  vinagre_tab_send_ctrlaltdel (vinagre_window_get_active_tab (window));
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

void
vinagre_cmd_view_scaling (GtkAction     *action,
			  VinagreWindow *window)
{
  gboolean active;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (active &&
      gdk_screen_is_composited (gtk_widget_get_screen (GTK_WIDGET (window))))
    {
      gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);
      vinagre_utils_show_error (_("Scaling does not work properly on composited windows. Disable the visual effects and try again."),
				GTK_WINDOW (window));
      return;
    }

  if (!vinagre_tab_set_scaling (vinagre_window_get_active_tab (window), active))
    {
      gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);
      vinagre_utils_show_error (_("Scaling is not supported on this installation.\n\nRead the README file (shipped with Vinagre) in order to know how to enable this feature."),
				GTK_WINDOW (window));
    }
}

void
vinagre_cmd_view_readonly (GtkAction     *action,
			   VinagreWindow *window)
{
  gboolean active;
  VinagreTab *tab;

  tab = vinagre_window_get_active_tab (window);
  if (!tab)
    return;

  active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  vinagre_tab_set_readonly (tab, active);
}

/* Bookmarks Menu */
void
vinagre_cmd_open_bookmark (VinagreWindow     *window,
			   VinagreConnection *conn)
{
  VinagreTab *tab;
  VinagreConnection *new_conn;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  tab = vinagre_window_conn_exists (window, conn);
  if (tab)
    {
      vinagre_window_set_active_tab (window, tab);
    }
  else
    {
      new_conn = vinagre_connection_clone (conn);
      tab = VINAGRE_TAB (vinagre_tab_new (new_conn, window));
      vinagre_notebook_add_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
				VINAGRE_TAB (tab),
				-1);
    }
}

void
vinagre_cmd_bookmarks_add (GtkAction     *action,
			   VinagreWindow *window)
{
  GtkWidget         *tab;
  VinagreConnection *conn;
  gchar             *name;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  tab = window->priv->active_tab;
  conn = vinagre_tab_get_conn (VINAGRE_TAB (tab));
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  vinagre_bookmarks_add (vinagre_bookmarks_get_default (),
                         conn,
                         GTK_WINDOW (window));

  if (window->priv->active_tab == tab)
    {
      name = vinagre_connection_get_best_name (conn);
      vinagre_tab_set_title (VINAGRE_TAB (window->priv->active_tab),
			     name);
      g_free (name);
    }
}

void
vinagre_cmd_bookmarks_edit (GtkAction     *action,
			    VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));
  g_return_if_fail (VINAGRE_IS_CONNECTION (window->priv->fav_conn_selected));

  vinagre_bookmarks_edit (vinagre_bookmarks_get_default (),
                          window->priv->fav_conn_selected,
                          GTK_WINDOW (window));
}

void
vinagre_cmd_bookmarks_del (GtkAction     *action,
			   VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));
  g_return_if_fail (VINAGRE_IS_CONNECTION (window->priv->fav_conn_selected));

  vinagre_bookmarks_del (vinagre_bookmarks_get_default (),
                         window->priv->fav_conn_selected,
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
    conn = window->priv->fav_conn_selected;

  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  vinagre_cmd_open_bookmark (window, conn);
}

/* Make url in about dialog clickable */
static void
vinagre_about_dialog_handle_url (GtkAboutDialog *about,
				 const char     *link,
				 gpointer        data)
{
  GError *error = NULL;
  gchar  *address, *command;

  VinagreWindow *window = VINAGRE_WINDOW (data);

  if (g_strstr_len (link, strlen (link), "@"))
    address = g_strdup_printf ("mailto:%s", link);
  else
    address = g_strdup (link);

  command = g_strconcat ("gnome-open ", address,  NULL);
	
  gdk_spawn_command_line_on_screen (gtk_window_get_screen (GTK_WINDOW (window)),
				    command,
				    &error);
  if (error != NULL) 
    {
      vinagre_utils_show_error (error->message, GTK_WINDOW (window));
      g_error_free (error);
    }

  g_free (command);
  g_free (address);
}

/* Help Menu */

void
vinagre_cmd_help_contents (GtkAction     *action,
			   VinagreWindow *window)
{
  GError *error = NULL;
  char *command;
  const char *lang;
  char *uri = NULL;
  int i;

  const char * const * langs = g_get_language_names ();

  for (i = 0; langs[i]; i++)
    {
      lang = langs[i];
      if (strchr (lang, '.')) 
          continue;

      if (uri)
	g_free (uri);

      uri = g_build_filename (DATADIR, "/gnome/help/vinagre/", lang, "/vinagre.xml", NULL);
					
      if (g_file_test (uri, G_FILE_TEST_EXISTS))
          break;
    }
	
  command = g_strconcat ("gnome-open ghelp://", uri,  NULL);
	
  gdk_spawn_command_line_on_screen (gtk_window_get_screen (GTK_WINDOW (window)),
				    command,
				    &error);
  if (error != NULL) 
    {
      vinagre_utils_show_error (error->message, GTK_WINDOW (window));
      g_error_free (error);
    }

  g_free (command);
  g_free (uri);
}

void
vinagre_cmd_help_about (GtkAction     *action,
			VinagreWindow *window)
{
  static const gchar * const authors[] = {
	"Jonh Wendell <jwendell@gnome.org>",
	NULL
  };

  static const gchar * const artists[] = {
	"Vinicius Depizzol <vdepizzol@gmail.com>",
	NULL
  };

  static const gchar copyright[] = \
	"Copyright \xc2\xa9 2007 Jonh Wendell";

  static const gchar comments[] = \
	N_("Vinagre is a VNC client for the GNOME Desktop");

  static const char *license[] = {
	N_("Vinagre is free software; you can redistribute it and/or modify "
	   "it under the terms of the GNU General Public License as published by "
	   "the Free Software Foundation; either version 2 of the License, or "
	   "(at your option) any later version."),
	N_("Vinagre is distributed in the hope that it will be useful, "
	   "but WITHOUT ANY WARRANTY; without even the implied warranty of "
	   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
	   "GNU General Public License for more details."),
	N_("You should have received a copy of the GNU General Public License "
	   "along with this program. If not, see <http://www.gnu.org/licenses/>.")
  };

  gchar *license_trans;

  license_trans = g_strjoin ("\n\n", _(license[0]), _(license[1]),
				     _(license[2]), NULL);

  /* Make URLs and email clickable in about dialog */
  gtk_about_dialog_set_url_hook (vinagre_about_dialog_handle_url, window, NULL);
  gtk_about_dialog_set_email_hook (vinagre_about_dialog_handle_url, window, NULL);


  gtk_show_about_dialog (GTK_WINDOW (window),
			 "authors", authors,
			 "artists", artists,
			 "comments", _(comments),
			 "copyright", copyright,
			 "license", license_trans,
			 "wrap-license", TRUE,
			 "logo-icon-name", "vinagre",
			 "translator-credits", _("translator-credits"),
			 "version", VERSION,
			 "website", "http://www.gnome.org/projects/vinagre/",
			 "website-label", _("Vinagre Website"),
			 NULL);
  g_free (license_trans);
}

/* vim: ts=8 */
