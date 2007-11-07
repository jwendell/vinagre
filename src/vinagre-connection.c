/*
 * vinagre-connection.c
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

#include <stdlib.h>
#include <glib/gi18n.h>
#include <libgnomevfs/gnome-vfs.h>

#include "vinagre-connection.h"
#include "vinagre-bookmarks.h"

VinagreConnection *
vinagre_connection_new ()
{
  VinagreConnection *conn;

  conn = g_new (VinagreConnection, 1);
  conn->host = NULL;
  conn->port = 0;
  conn->name = NULL;
  conn->password = NULL;
  conn->desktop_name = NULL;

  return conn;
}

void 
vinagre_connection_set_host (VinagreConnection *conn,
			     const char *host)
{
  if (conn->host)
    g_free (conn->host);
  conn->host = g_strdup (host);
}

void
vinagre_connection_set_port (VinagreConnection *conn,
			     int port)
{
  conn->port = port;
}

void
vinagre_connection_set_password (VinagreConnection *conn,
				 const char *password)
{
  if (conn->password)
    g_free (conn->password);
  conn->password = g_strdup (password);
}

void 
vinagre_connection_set_name (VinagreConnection *conn,
			     const char *name)
{
  if (conn->name)
    g_free (conn->name);
  conn->name = g_strdup (name);
}

void 
vinagre_connection_set_desktop_name (VinagreConnection *conn,
				     const char *desktop_name)
{
  if (conn->desktop_name)
    g_free (conn->desktop_name);
  conn->desktop_name = g_strdup (desktop_name);
}

void
vinagre_connection_free (VinagreConnection *conn)
{
  if (conn) {

    if (conn->host)
      g_free (conn->host);
    conn->host = NULL;

    if (conn->password)
      g_free (conn->password);
    conn->password = NULL;

    if (conn->name)
      g_free (conn->name);
    conn->name = NULL;

    if (conn->desktop_name)
      g_free (conn->desktop_name);
    conn->desktop_name = NULL;

    g_free (conn);
    conn = NULL;
  }
}

gchar *
vinagre_connection_best_name (VinagreConnection *conn)
{
  g_return_val_if_fail (conn != NULL, NULL);

  if (conn->name)
    return g_strdup (conn->name);

  if (conn->desktop_name)
    return g_strdup (conn->desktop_name);

  if (conn->host)
    return g_strdup_printf ("%s:%d", conn->host, conn->port);

  return NULL;
}

VinagreConnection *
vinagre_connection_clone (VinagreConnection *conn)
{
  VinagreConnection *new_conn;

  new_conn = vinagre_connection_new ();

  vinagre_connection_set_host (new_conn, conn->host);
  vinagre_connection_set_port (new_conn, conn->port);
  vinagre_connection_set_password (new_conn, conn->password);
  vinagre_connection_set_name (new_conn, conn->name);
  vinagre_connection_set_desktop_name (new_conn, conn->desktop_name);

  return new_conn;
}

VinagreConnection *
vinagre_connection_new_from_string (const gchar *url)
{
  VinagreConnection *conn;
  gchar **server;
  gint    port;
  gchar  *host;

  server = g_strsplit (url, ":", 2);
  host = server[0];
  port = server[1] ? atoi (server[1]) : 5900;

  conn = vinagre_bookmarks_exists (host, port);
  if (!conn)
    {
      conn = vinagre_connection_new ();
      vinagre_connection_set_host (conn, host);
      vinagre_connection_set_port (conn, port);
    }

  g_strfreev (server);
  return conn;
}

VinagreConnection *
vinagre_connection_new_from_file (const gchar *uri, gchar **error_msg)
{
  GKeyFile          *file;
  GError            *error = NULL;
  gboolean           loaded;
  VinagreConnection *conn = NULL;
  gchar             *host = NULL;
  gint               port;
  GnomeVFSResult     result;
  int                file_size;
  char              *data = NULL;

  result = gnome_vfs_read_entire_file (uri, &file_size, &data);
  if (result != GNOME_VFS_OK)
    {
      *error_msg = g_strdup (gnome_vfs_result_to_string (result));

      if (data)
	g_free (data);

      return NULL;
    }

  file = g_key_file_new ();
  loaded = g_key_file_load_from_data (file,
				      data,
				      file_size,
				      G_KEY_FILE_NONE,
				      &error);
  if (loaded)
    {
      host = g_key_file_get_string (file, "connection", "host", NULL);
      port = g_key_file_get_integer (file, "connection", "port", NULL);
      if (host)
	{
	  conn = vinagre_bookmarks_exists (host, port);
	  if (!conn)
	    {
	      conn = vinagre_connection_new ();
	      vinagre_connection_set_host (conn, host);
	      vinagre_connection_set_port (conn, port);
	    }
	  g_free (host);
	}
    }
  else
    {
      if (error)
	{
	  *error_msg = g_strdup (error->message);
	  g_error_free (error);
	}
    }

  if (data)
    g_free (data);

  g_key_file_free (file);
  *error_msg = NULL;

  return conn;
}
/* vim: ts=8 */
