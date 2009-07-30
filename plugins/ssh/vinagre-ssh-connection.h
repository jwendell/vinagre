/*
 * vinagre-ssh-connection.h
 * Child class of abstract VinagreConnection, specific to SSH protocol
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

#ifndef __VINAGRE_SSH_CONNECTION_H__
#define __VINAGRE_SSH_CONNECTION_H__

#include <vinagre/vinagre-connection.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_SSH_CONNECTION             (vinagre_ssh_connection_get_type ())
#define VINAGRE_SSH_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_SSH_CONNECTION, VinagreSshConnection))
#define VINAGRE_SSH_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_SSH_CONNECTION, VinagreSshConnectionClass))
#define VINAGRE_IS_SSH_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_SSH_CONNECTION))
#define VINAGRE_IS_SSH_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_SSH_CONNECTION))
#define VINAGRE_SSH_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_SSH_CONNECTION, VinagreSshConnectionClass))

typedef struct _VinagreSshConnectionClass   VinagreSshConnectionClass;
typedef struct _VinagreSshConnection        VinagreSshConnection;
typedef struct _VinagreSshConnectionPrivate VinagreSshConnectionPrivate;

struct _VinagreSshConnectionClass
{
  VinagreConnectionClass parent_class;
};

struct _VinagreSshConnection
{
  VinagreConnection parent_instance;
  VinagreSshConnectionPrivate *priv;
};


GType vinagre_ssh_connection_get_type (void) G_GNUC_CONST;

VinagreConnection*  vinagre_ssh_connection_new (void);

G_END_DECLS

#endif /* __VINAGRE_SSH_CONNECTION_H__  */
/* vim: set ts=8: */
