/*
 * vinagre-ssh-tab.h
 * SSH Tab
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

#ifndef __VINAGRE_SSH_TAB_H__
#define __VINAGRE_SSH_TAB_H__

#include <vinagre/vinagre-tab.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_SSH_TAB              (vinagre_ssh_tab_get_type())
#define VINAGRE_SSH_TAB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_SSH_TAB, VinagreSshTab))
#define VINAGRE_SSH_TAB_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_SSH_TAB, VinagreSshTab const))
#define VINAGRE_SSH_TAB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_SSH_TAB, VinagreSshTabClass))
#define VINAGRE_IS_SSH_TAB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_SSH_TAB))
#define VINAGRE_IS_SSH_TAB_CLASS(klass)...(G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_SSH_TAB))
#define VINAGRE_SSH_TAB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_SSH_TAB, VinagreSshTabClass))

typedef struct _VinagreSshTabPrivate VinagreSshTabPrivate;
typedef struct _VinagreSshTab        VinagreSshTab;
typedef struct _VinagreSshTabClass   VinagreSshTabClass;


struct _VinagreSshTab 
{
  VinagreTab tab;
  VinagreSshTabPrivate *priv;
};

struct _VinagreSshTabClass 
{
  VinagreTabClass parent_class;
};

GType		vinagre_ssh_tab_get_type		(void) G_GNUC_CONST;

GtkWidget *	vinagre_ssh_tab_new 			(VinagreConnection *conn,
							 VinagreWindow     *window);

G_END_DECLS

#endif  /* __VINAGRE_SSH_TAB_H__  */
/* vim: set ts=8: */
