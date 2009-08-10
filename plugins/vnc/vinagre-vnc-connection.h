/*
 * vinagre-vnc-connection.h
 * Child class of abstract VinagreConnection, specific to VNC protocol
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

#ifndef __VINAGRE_VNC_CONNECTION_H__
#define __VINAGRE_VNC_CONNECTION_H__

#include <vinagre/vinagre-connection.h>

G_BEGIN_DECLS

gboolean scaling_command_line;

#define VINAGRE_TYPE_VNC_CONNECTION             (vinagre_vnc_connection_get_type ())
#define VINAGRE_VNC_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_VNC_CONNECTION, VinagreVncConnection))
#define VINAGRE_VNC_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_VNC_CONNECTION, VinagreVncConnectionClass))
#define VINAGRE_IS_VNC_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_VNC_CONNECTION))
#define VINAGRE_IS_VNC_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_VNC_CONNECTION))
#define VINAGRE_VNC_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_VNC_CONNECTION, VinagreVncConnectionClass))

typedef struct _VinagreVncConnectionClass   VinagreVncConnectionClass;
typedef struct _VinagreVncConnection        VinagreVncConnection;
typedef struct _VinagreVncConnectionPrivate VinagreVncConnectionPrivate;

struct _VinagreVncConnectionClass
{
  VinagreConnectionClass parent_class;
};

struct _VinagreVncConnection
{
  VinagreConnection parent_instance;
  VinagreVncConnectionPrivate *priv;
};


GType vinagre_vnc_connection_get_type (void) G_GNUC_CONST;

VinagreConnection*  vinagre_vnc_connection_new (void);

const gchar*	    vinagre_vnc_connection_get_desktop_name (VinagreVncConnection *conn);
void		    vinagre_vnc_connection_set_desktop_name (VinagreVncConnection *conn,
							     const gchar *desktop_name);

gboolean	    vinagre_vnc_connection_get_view_only    (VinagreVncConnection *conn);
void		    vinagre_vnc_connection_set_view_only    (VinagreVncConnection *conn,
							     gboolean value);

gboolean	    vinagre_vnc_connection_get_scaling      (VinagreVncConnection *conn);
void		    vinagre_vnc_connection_set_scaling      (VinagreVncConnection *conn,
							     gboolean value);

gint		    vinagre_vnc_connection_get_shared       (VinagreVncConnection *conn);
void		    vinagre_vnc_connection_set_shared       (VinagreVncConnection *conn,
							     gint value);

G_END_DECLS

#endif /* __VINAGRE_VNC_CONNECTION_H__  */
/* vim: set ts=8: */
