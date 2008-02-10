/*
 * vinagre-connection.h
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

#ifndef __VINAGRE_CONNECTION_H__
#define __VINAGRE_CONNECTION_H__

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef enum
{
  VINAGRE_CONNECTION_TYPE_VNC = 0
} VinagreConnectionType;

typedef struct
{
  char *host;
  int   port;
  char *name;
  char *password;
  char *desktop_name;
  VinagreConnectionType type;
} VinagreConnection;

VinagreConnection *vinagre_connection_new (void);

void		   vinagre_connection_set_host		(VinagreConnection *conn,
							 const char *host);
void		   vinagre_connection_set_port		(VinagreConnection *conn,
							 int port);
void		   vinagre_connection_set_password	(VinagreConnection *conn,
							 const char *password);
void		   vinagre_connection_set_name		(VinagreConnection *conn,
							 const char *name);
void		   vinagre_connection_set_desktop_name	(VinagreConnection *conn,
							 const char *desktop_name);

void               vinagre_connection_free		(VinagreConnection *conn);

gchar             *vinagre_connection_best_name		(VinagreConnection *conn);

VinagreConnection *vinagre_connection_clone		(VinagreConnection *conn);

VinagreConnection *vinagre_connection_new_from_string	(const gchar *url, gchar **error_msg);
VinagreConnection *vinagre_connection_new_from_file	(const gchar *uri, gchar **error_msg);

GdkPixbuf         *vinagre_connection_get_icon		(VinagreConnection *conn);

#endif /* __VINAGRE_CONNECTION_H__  */
/* vim: ts=8 */
