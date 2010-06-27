/*
 * vinagre-rdp-connection.h
 * Child class of abstract VinagreConnection, specific to RDP protocol
 * This file is part of vinagre
 *
 * Copyright (C) 2010 - Jonh Wendell <wendell@bani.com.br>
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

#ifndef __VINAGRE_RDP_CONNECTION_H__
#define __VINAGRE_RDP_CONNECTION_H__

#include <vinagre/vinagre-connection.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_RDP_CONNECTION             (vinagre_rdp_connection_get_type ())
#define VINAGRE_RDP_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_RDP_CONNECTION, VinagreRdpConnection))
#define VINAGRE_RDP_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_RDP_CONNECTION, VinagreRdpConnectionClass))
#define VINAGRE_IS_RDP_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_RDP_CONNECTION))
#define VINAGRE_IS_RDP_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_RDP_CONNECTION))
#define VINAGRE_RDP_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_RDP_CONNECTION, VinagreRdpConnectionClass))

typedef struct _VinagreRdpConnectionClass   VinagreRdpConnectionClass;
typedef struct _VinagreRdpConnection        VinagreRdpConnection;
typedef struct _VinagreRdpConnectionPrivate VinagreRdpConnectionPrivate;

struct _VinagreRdpConnectionClass
{
  VinagreConnectionClass parent_class;
};

struct _VinagreRdpConnection
{
  VinagreConnection parent_instance;
  VinagreRdpConnectionPrivate *priv;
};


GType vinagre_rdp_connection_get_type (void) G_GNUC_CONST;

VinagreConnection*  vinagre_rdp_connection_new (void);

G_END_DECLS

#endif /* __VINAGRE_RDP_CONNECTION_H__  */
/* vim: set ts=8: */
