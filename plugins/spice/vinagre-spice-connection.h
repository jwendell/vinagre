/*
 * vinagre-spice-connection.h
 * Child class of abstract VinagreConnection, specific to SPICE protocol
 * This file is part of vinagre
 *
 * Copyright (C) 2011 Red Hat, Inc.
 * Author: Marc-Andre Lureau <marcandre.lureau@redhat.com>
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

#ifndef __VINAGRE_SPICE_CONNECTION_H__
#define __VINAGRE_SPICE_CONNECTION_H__

#include <vinagre/vinagre-connection.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_SPICE_CONNECTION             (vinagre_spice_connection_get_type ())
#define VINAGRE_SPICE_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_SPICE_CONNECTION, VinagreSpiceConnection))
#define VINAGRE_SPICE_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_SPICE_CONNECTION, VinagreSpiceConnectionClass))
#define VINAGRE_IS_SPICE_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_SPICE_CONNECTION))
#define VINAGRE_IS_SPICE_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_SPICE_CONNECTION))
#define VINAGRE_SPICE_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_SPICE_CONNECTION, VinagreSpiceConnectionClass))

typedef struct _VinagreSpiceConnectionClass   VinagreSpiceConnectionClass;
typedef struct _VinagreSpiceConnection        VinagreSpiceConnection;
typedef struct _VinagreSpiceConnectionPrivate VinagreSpiceConnectionPrivate;

struct _VinagreSpiceConnectionClass
{
  VinagreConnectionClass parent_class;
};

struct _VinagreSpiceConnection
{
  VinagreConnection parent_instance;
  VinagreSpiceConnectionPrivate *priv;
};


GType vinagre_spice_connection_get_type (void) G_GNUC_CONST;

VinagreConnection*  vinagre_spice_connection_new (void);

const gchar*	    vinagre_spice_connection_get_desktop_name (VinagreSpiceConnection *conn);
void		    vinagre_spice_connection_set_desktop_name (VinagreSpiceConnection *conn,
							       const gchar *desktop_name);

gboolean	    vinagre_spice_connection_get_view_only    (VinagreSpiceConnection *conn);
void		    vinagre_spice_connection_set_view_only    (VinagreSpiceConnection *conn,
							       gboolean value);

gboolean	    vinagre_spice_connection_get_scaling      (VinagreSpiceConnection *conn);
void		    vinagre_spice_connection_set_scaling      (VinagreSpiceConnection *conn,
							       gboolean value);

gboolean	    vinagre_spice_connection_get_resize_guest (VinagreSpiceConnection *conn);
void		    vinagre_spice_connection_set_resize_guest (VinagreSpiceConnection *conn,
							       gboolean value);

gint		    vinagre_spice_connection_get_fd	      (VinagreSpiceConnection *conn);
void		    vinagre_spice_connection_set_fd	      (VinagreSpiceConnection *conn,
							       gint value);

GSocket *	    vinagre_spice_connection_get_socket	      (VinagreSpiceConnection *conn);
void		    vinagre_spice_connection_set_socket	      (VinagreSpiceConnection *conn,
							       GSocket *socket);

const gchar*	    vinagre_spice_connection_get_ssh_tunnel_host (VinagreSpiceConnection *conn);
void		    vinagre_spice_connection_set_ssh_tunnel_host (VinagreSpiceConnection *conn,
								  const gchar *host);

gboolean	    vinagre_spice_connection_get_auto_clipboard (VinagreSpiceConnection *conn);
void		    vinagre_spice_connection_set_auto_clipboard (VinagreSpiceConnection *conn,
								 gboolean value);

G_END_DECLS

#endif /* __VINAGRE_SPICE_CONNECTION_H__  */
/* vim: set ts=8: */
