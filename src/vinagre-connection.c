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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#include "vinagre-connection.h"

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

const gchar *
vinagre_connection_best_name (VinagreConnection *conn)
{
  g_return_val_if_fail (conn != NULL, NULL);

  if (conn->name)
    return conn->name;

  if (conn->desktop_name)
    return conn->desktop_name;

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

  return new_conn;
}
