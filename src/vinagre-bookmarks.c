/*
 * vinagre-bookmarks.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008  Jonh Wendell <wendell@bani.com.br>
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

#include "vinagre-bookmarks.h"
#include "vinagre-utils.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <glade/glade.h>
#include <string.h>

struct _VinagreBookmarksPrivate
{
  GKeyFile     *file;
  gchar        *filename;
  GSList       *conns;
  GFileMonitor *monitor;
};

enum
{
  BOOKMARK_CHANGED,
  LAST_SIGNAL
};

#define VINAGRE_BOOKMARKS_FILE  "vinagre.bookmarks"

G_DEFINE_TYPE (VinagreBookmarks, vinagre_bookmarks, G_TYPE_OBJECT);

static VinagreBookmarks *book_singleton = NULL;
static guint signals[LAST_SIGNAL] = { 0 };

/* Prototypes */
static void vinagre_bookmarks_update_file  (VinagreBookmarks *book);
static void vinagre_bookmarks_update_conns (VinagreBookmarks *book);
static void vinagre_bookmarks_save_file    (VinagreBookmarks *book);
static void vinagre_bookmarks_clear_conns  (VinagreBookmarks *book);
static void vinagre_bookmarks_file_changed (GFileMonitor     *monitor,
		                            GFile             *file,
		                            GFile             *other_file,
		                            GFileMonitorEvent  event_type,
		                            VinagreBookmarks  *book);


static void
vinagre_bookmarks_init (VinagreBookmarks *book)
{
  GFile *gfile;

  book->priv = G_TYPE_INSTANCE_GET_PRIVATE (book, VINAGRE_TYPE_BOOKMARKS, VinagreBookmarksPrivate);

  book->priv->conns = NULL;
  book->priv->file = NULL;

  book->priv->filename = g_build_filename (g_get_user_data_dir (),
			                   "vinagre",
			                   VINAGRE_BOOKMARKS_FILE,
			                   NULL);
  gfile = g_file_new_for_path (book->priv->filename);

  if (!g_file_test (book->priv->filename, G_FILE_TEST_EXISTS))
    {
      gchar *old;

      old = g_build_filename (g_get_home_dir (),
			      ".gnome2",
			      VINAGRE_BOOKMARKS_FILE,
			      NULL);
      if (g_file_test (old, G_FILE_TEST_EXISTS))
	{
	  GFile *src;
	  GError *error = NULL;

	  g_message (_("Copying the bookmarks file to the new location. This operation is only supposed to run once."));
	  src = g_file_new_for_path (old);

	  if (!g_file_copy (src, gfile, G_FILE_COPY_NONE, NULL, NULL, NULL, &error))
	    {
	      g_warning (_("Error: %s"), error->message);
	      g_error_free (error);
	    }

	  g_object_unref (src);
	}

      g_free (old);
    }

  vinagre_bookmarks_update_file (book);
  vinagre_bookmarks_update_conns (book);

  book->priv->monitor = g_file_monitor_file (gfile,
                                             G_FILE_MONITOR_NONE,
                                             NULL,
                                             NULL);
  g_signal_connect (book->priv->monitor,
                    "changed",
                    G_CALLBACK (vinagre_bookmarks_file_changed),
                    book);
  g_object_unref (gfile);
}

static void
vinagre_bookmarks_finalize (GObject *object)
{
  VinagreBookmarks *book = VINAGRE_BOOKMARKS (object);

  g_key_file_free (book->priv->file);
  book->priv->file = NULL;
  vinagre_bookmarks_clear_conns (book);

  g_free (book->priv->filename);
  book->priv->filename = NULL;

  g_file_monitor_cancel (book->priv->monitor);
  g_object_unref (book->priv->monitor);

  G_OBJECT_CLASS (vinagre_bookmarks_parent_class)->finalize (object);
}

static void
vinagre_bookmarks_class_init (VinagreBookmarksClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  GObjectClass* parent_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreBookmarksPrivate));

  object_class->finalize = vinagre_bookmarks_finalize;

  signals[BOOKMARK_CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreBookmarksClass, changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
}

VinagreBookmarks *
vinagre_bookmarks_get_default (void)
{
  if (G_UNLIKELY (!book_singleton))
    book_singleton = VINAGRE_BOOKMARKS (g_object_new (VINAGRE_TYPE_BOOKMARKS,
                                                      NULL));
  return book_singleton;
}

static VinagreConnection *
vinagre_bookmarks_find_conn (VinagreBookmarks  *book,
                             VinagreConnection *conn)
{
  GSList *l, *next;

  for (l = book->priv->conns; l; l = next)
    {
      VinagreConnection *local = VINAGRE_CONNECTION (l->data);

      if ( (g_str_equal (vinagre_connection_get_host (conn),
                         vinagre_connection_get_host (local)))
          &&
            (vinagre_connection_get_port (conn) == vinagre_connection_get_port (local) ) )
        return local;

      next = l->next;
    }

  return NULL;
}

static void
vinagre_bookmarks_add_conn (VinagreBookmarks  *book,
                            VinagreConnection *conn)
{
  book->priv->conns = g_slist_prepend (book->priv->conns,
                                       vinagre_connection_clone (conn));
  vinagre_bookmarks_save_file (book);
}

static void
vinagre_bookmarks_edit_conn (VinagreBookmarks  *book,
                             VinagreConnection *old_conn,
                             VinagreConnection *conn)
{
  VinagreConnection *local = vinagre_bookmarks_find_conn (book, old_conn);

  g_return_if_fail (VINAGRE_IS_CONNECTION (local));

  g_object_unref (local);
  book->priv->conns = g_slist_remove (book->priv->conns,
                                      local);
  book->priv->conns = g_slist_prepend (book->priv->conns,
                                       vinagre_connection_clone (conn));
  
  vinagre_bookmarks_save_file (book);
}

static void
vinagre_bookmarks_del_conn (VinagreBookmarks  *book,
                            VinagreConnection *conn)
{
  VinagreConnection *local = vinagre_bookmarks_find_conn (book, conn);

  g_return_if_fail (VINAGRE_IS_CONNECTION (local));

  book->priv->conns = g_slist_remove (book->priv->conns,
                                      local);
  g_object_unref (local);
  
  vinagre_bookmarks_save_file (book);
}

static void
update_group_from_conn (VinagreBookmarks  *book,
			VinagreConnection *conn,
			const gchar *group)
{
  g_key_file_set_string (book->priv->file,
			 group,
			 "host",
			 vinagre_connection_get_host (conn));
  g_key_file_set_integer (book->priv->file,
			  group,
			  "port",
			  vinagre_connection_get_port (conn));
  g_key_file_set_boolean (book->priv->file,
			  group,
			  "view_only",
			  vinagre_connection_get_view_only (conn));
  g_key_file_set_boolean (book->priv->file,
			  group,
			  "scaling",
			  vinagre_connection_get_scaling (conn));
  g_key_file_set_boolean (book->priv->file,
			  group,
			  "fullscreen",
			  vinagre_connection_get_fullscreen (conn));
}

gboolean
vinagre_bookmarks_add (VinagreBookmarks  *book,
                       VinagreConnection *conn,
		       GtkWindow         *window)
{
  gint result;
  GladeXML    *xml;
  const gchar *glade_file;
  GtkWidget   *dialog;
  const gchar *name;

  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS (book), FALSE);
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), FALSE);

  g_object_ref (conn);

  glade_file = vinagre_utils_get_glade_filename ();
  xml = glade_xml_new (glade_file, "add_to_bookmarks_dialog", NULL);
  dialog = glade_xml_get_widget (xml, "add_to_bookmarks_dialog");
  gtk_window_set_transient_for (GTK_WINDOW(dialog), window);

  gtk_widget_show_all (dialog);
 
  result = gtk_dialog_run (GTK_DIALOG (dialog));

  if (result == GTK_RESPONSE_OK)
    {
      name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "bookmark_name_entry")));
      if (strlen(name) < 1)
	name = vinagre_connection_get_host (conn);

      update_group_from_conn (book, conn, name);

      vinagre_connection_set_name (conn, name);
      vinagre_bookmarks_add_conn (book, conn);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
  g_object_unref (G_OBJECT (xml));
  g_object_unref (conn);

  return (result == GTK_RESPONSE_OK);
}

gboolean
vinagre_bookmarks_edit (VinagreBookmarks  *book,
                        VinagreConnection *conn,
		        GtkWindow         *window)
{
  gint result;
  GladeXML    *xml;
  const gchar *glade_file;
  GtkWidget   *dialog, *host_entry, *name_entry;
  GtkWidget   *fs_check, *sc_check, *vo_check;
  gchar       *str;

  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS (book), FALSE);
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), FALSE);

  glade_file = vinagre_utils_get_glade_filename ();
  xml = glade_xml_new (glade_file, "edit_bookmark_dialog", NULL);
  dialog = glade_xml_get_widget (xml, "edit_bookmark_dialog");
  gtk_window_set_transient_for (GTK_WINDOW(dialog), window);

  name_entry = glade_xml_get_widget (xml, "edit_bookmark_name_entry");
  host_entry = glade_xml_get_widget (xml, "edit_bookmark_host_entry");
  fs_check   = glade_xml_get_widget (xml, "edit_fullscreen_check");
  sc_check   = glade_xml_get_widget (xml, "edit_scaling_check");
  vo_check   = glade_xml_get_widget (xml, "edit_viewonly_check");

  gtk_entry_set_text (GTK_ENTRY(name_entry), vinagre_connection_get_name (conn));
  str = vinagre_connection_get_string_rep (conn, FALSE);
  gtk_entry_set_text (GTK_ENTRY(host_entry), str);
  g_free (str);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fs_check),
				vinagre_connection_get_fullscreen (conn));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sc_check),
				vinagre_connection_get_scaling (conn));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (vo_check),
				vinagre_connection_get_view_only (conn));

  gtk_widget_show_all (dialog);
 
  result = gtk_dialog_run (GTK_DIALOG (dialog));

  if (result == GTK_RESPONSE_OK)
    {
      const gchar *name;
      gchar *host, *error_str;
      gint port;
      VinagreConnection *old_conn = vinagre_connection_clone (conn);

      g_key_file_remove_group (book->priv->file, vinagre_connection_get_name (conn), NULL);

      name = gtk_entry_get_text (GTK_ENTRY (name_entry));
      if (vinagre_connection_split_string (gtk_entry_get_text (GTK_ENTRY (host_entry)),
					   &host,
					   &port,
					   &error_str))
	{
	  vinagre_connection_set_host (conn, host);
	  vinagre_connection_set_port (conn, port);
	  vinagre_connection_set_view_only (conn,
					    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (vo_check)));
	  vinagre_connection_set_scaling (conn,
					  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sc_check)));
	  vinagre_connection_set_fullscreen (conn,
					     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fs_check)));

	  if (strlen (name) < 1)
	    if (strlen (vinagre_connection_get_name (conn)) < 1)
	      name = vinagre_connection_get_host (conn);
	    else
	      name = vinagre_connection_get_name (conn);

	  update_group_from_conn (book, conn, name);

	  vinagre_connection_set_name (conn, name);
	  vinagre_bookmarks_edit_conn (book, old_conn, conn);
	}
      else
	{
	  vinagre_utils_show_error (error_str, window);
	  g_free (error_str);
	}

      g_object_unref (old_conn);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
  g_object_unref (G_OBJECT (xml));

  return (result == GTK_RESPONSE_OK);
}

gboolean
vinagre_bookmarks_del (VinagreBookmarks  *book,
                       VinagreConnection *conn,
		       GtkWindow         *window)
{
  gint       result;
  GtkWidget *dialog;
  gchar     *name;
  GError    *error = NULL;

  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS (book), FALSE);
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), FALSE);

  name = vinagre_connection_get_best_name (conn);
  if (!g_key_file_has_group (book->priv->file, name))
    {
      g_free (name);
      return FALSE;
    }

  dialog = gtk_message_dialog_new (window,
				   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_QUESTION,
				   GTK_BUTTONS_OK_CANCEL,
				   _("Confirm removal?"));

  gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (dialog),
					    _("Are you sure you want to remove <i>%s</i> from bookmarks?"),
					    name);
 
  result = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  if (result == GTK_RESPONSE_OK)
    {
      g_key_file_remove_group (book->priv->file, name, &error);
      if (error)
	{
	  g_warning (_("Error while removing %s from bookmarks: %s"),
			name,
			error->message);
	  g_error_free (error);
	  g_free (name);
	  return FALSE;
	}
      vinagre_bookmarks_del_conn (book, conn);
    }

  g_free (name);
  return (result == GTK_RESPONSE_OK);
}

VinagreConnection *
vinagre_bookmarks_exists (VinagreBookmarks *book,
                          const gchar *host,
                          gint port)
{
  VinagreConnection *conn = NULL;
  GSList *l, *next;

  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS (book), NULL);

  for (l = book->priv->conns; l; l = next)
    {
      VinagreConnection *con = VINAGRE_CONNECTION (l->data);

      if ( (g_str_equal (host, vinagre_connection_get_host (con))) &&
            (port == vinagre_connection_get_port (con) ) )
        {
          conn = vinagre_connection_clone (con);
          break;
        }
      next = l->next;
    }
  
  return conn;
}

GSList *
vinagre_bookmarks_get_all (VinagreBookmarks *book)
{
  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS (book), NULL);

  return book->priv->conns;
}

static void
vinagre_bookmarks_update_file (VinagreBookmarks *book)
{
  gboolean loaded = TRUE;
  GError   *error = NULL;

  if (book->priv->file)
    g_key_file_free (book->priv->file);
  book->priv->file = g_key_file_new ();

  if (g_file_test (book->priv->filename, G_FILE_TEST_EXISTS))
    loaded = g_key_file_load_from_file (book->priv->file,
					book->priv->filename,
					G_KEY_FILE_NONE,
					&error);
  if (!loaded)
    {
      if (error)
	{
	  g_warning (_("Error while initializing bookmarks: %s"), error->message);
	  g_error_free (error);
	}
    }
}

static void
vinagre_bookmarks_save_file (VinagreBookmarks *book)
{
  gchar    *data;
  gsize    length;
  GError   *error;

  error = NULL;
  data = g_key_file_to_data (book->priv->file,
			     &length,
			     &error);
  if (!data)
    {
      if (error)
	{
	  g_warning (_("Error while saving bookmarks: %s"), error->message);
	  g_error_free (error);
	}

      return;

    }

  error = NULL;

  if (!g_file_set_contents (book->priv->filename,
			    data,
			    length,
			    &error))
    {
      if (error)
	{
	  g_warning (_("Error while saving bookmarks: %s"), error->message);
	  g_error_free (error);
          g_free (data);
          return;
	}
    }

  g_free (data);
}

static void
vinagre_bookmarks_clear_conns (VinagreBookmarks *book)
{
  g_slist_foreach (book->priv->conns, (GFunc) g_object_unref, NULL);
  g_slist_free (book->priv->conns);

  book->priv->conns = NULL;
}

static void
vinagre_bookmarks_update_conns (VinagreBookmarks *book)
{
  gsize length, i;
  gchar **conns;

  vinagre_bookmarks_clear_conns (book);

  conns = g_key_file_get_groups (book->priv->file, &length);
  for (i=0; i<length; i++)
    {
      VinagreConnection *conn;
      gchar             *s_value;
      gint               i_value;
      gboolean           b_value;

      s_value = g_key_file_get_string (book->priv->file, conns[i], "host", NULL);
      if (!s_value)
        continue;

      conn = vinagre_connection_new ();
      vinagre_connection_set_name (conn, conns[i]);
      vinagre_connection_set_host (conn, s_value);
      g_free (s_value);

      i_value = g_key_file_get_integer (book->priv->file, conns[i], "port", NULL);
      if (i_value == 0)
        i_value = 5900;
      vinagre_connection_set_port (conn, i_value);

      b_value = g_key_file_get_boolean (book->priv->file, conns[i], "view_only", NULL);
      vinagre_connection_set_view_only (conn, b_value);

      b_value = g_key_file_get_boolean (book->priv->file, conns[i], "fullscreen", NULL);
      vinagre_connection_set_fullscreen (conn, b_value);

      b_value = g_key_file_get_boolean (book->priv->file, conns[i], "scaling", NULL);
      vinagre_connection_set_scaling (conn, b_value);

      book->priv->conns = g_slist_prepend (book->priv->conns, conn);
    }

  g_strfreev (conns);
}

static void
vinagre_bookmarks_file_changed (GFileMonitor      *monitor,
		                GFile             *file,
		                GFile             *other_file,
		                GFileMonitorEvent  event_type,
		                VinagreBookmarks  *book)
{
  if (event_type != G_FILE_MONITOR_EVENT_CHANGED &&
      event_type != G_FILE_MONITOR_EVENT_CREATED &&
      event_type != G_FILE_MONITOR_EVENT_DELETED)
    return;

  vinagre_bookmarks_update_file (book);
  vinagre_bookmarks_update_conns (book);

  g_signal_emit (book, signals[BOOKMARK_CHANGED], 0);
}

/* vim: ts=8 */
