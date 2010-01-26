/*
 * vinagre-vnc-tunnel.h
 * VNC SSH Tunneling for Vinagre
 * This file is part of vinagre
 *
 * Copyright (C) 2009 - Jonh Wendell <wendell@bani.com.br>
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

#ifndef __VINAGRE_VNC_TUNNEL_H__
#define __VINAGRE_VNC_TUNNEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum
{
  VINAGRE_VNC_TUNNEL_ERROR_NO_FREE_PORT = 1
} VinagreVncTunnelError;

#define VINAGRE_VNC_TUNNEL_ERROR vinagre_vnc_tunnel_error_quark()
GQuark vinagre_vnc_tunnel_error_quark (void);

gboolean vinagre_vnc_tunnel_create (GtkWindow *parent,
				    gchar **original_host,
				    gchar **original_port,
				    gchar *gateway,
				    GError **error);

G_END_DECLS

#endif  /* __VINAGRE_VNC_TUNNEL_H__  */
/* vim: set ts=8: */
