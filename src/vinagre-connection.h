/*
 * vinagre-connection.h
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

#ifndef __VINAGRE_CONNECTION_H__
#define __VINAGRE_CONNECTION_H__

#include <glib.h>

typedef struct
{
  char *host;
  int   port;
  char *name;
  char *password;
  char *desktop_name;
  int   sock;
} VinagreConnection;

VinagreConnection *vinagre_connection_new (void);

void		  vinagre_connection_set_host (VinagreConnection *conn,
					       const char *host);
void		  vinagre_connection_set_port (VinagreConnection *conn,
					       int port);
void		  vinagre_connection_set_password (VinagreConnection *conn,
						   const char *password);
void		  vinagre_connection_set_name (VinagreConnection *conn,
					       const char *name);

void              vinagre_connection_free (VinagreConnection *conn);
gboolean          vinagre_connection_connect (VinagreConnection *conn);
const gchar       *vinagre_connection_best_name (VinagreConnection *conn);

VinagreConnection *vinagre_connection_clone (VinagreConnection *conn);

#endif /* __VINAGRE_CONNECTION_H__  */
