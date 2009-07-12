/*
 * vinagre-vnc-tab.h
 * VNC Tab
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

#ifndef __VINAGRE_VNC_TAB_H__
#define __VINAGRE_VNC_TAB_H__

#include <vinagre/vinagre-tab.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_VNC_TAB              (vinagre_vnc_tab_get_type())
#define VINAGRE_VNC_TAB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_VNC_TAB, VinagreVncTab))
#define VINAGRE_VNC_TAB_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_VNC_TAB, VinagreVncTab const))
#define VINAGRE_VNC_TAB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_VNC_TAB, VinagreVncTabClass))
#define VINAGRE_IS_VNC_TAB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_VNC_TAB))
#define VINAGRE_IS_VNC_TAB_CLASS(klass)...(G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_VNC_TAB))
#define VINAGRE_VNC_TAB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_VNC_TAB, VinagreVncTabClass))

typedef struct _VinagreVncTabPrivate VinagreVncTabPrivate;
typedef struct _VinagreVncTab        VinagreVncTab;
typedef struct _VinagreVncTabClass   VinagreVncTabClass;


struct _VinagreVncTab 
{
  VinagreTab tab;
  VinagreVncTabPrivate *priv;
};

struct _VinagreVncTabClass 
{
  VinagreTabClass parent_class;
};

GType		vinagre_vnc_tab_get_type		(void) G_GNUC_CONST;

GtkWidget *	vinagre_vnc_tab_new 			(VinagreConnection *conn,
							 VinagreWindow     *window);

void		vinagre_vnc_tab_send_ctrlaltdel		(VinagreVncTab *tab);
void		vinagre_vnc_tab_paste_text		(VinagreVncTab *tab,
							 const gchar   *text);

gboolean	vinagre_vnc_tab_set_scaling		(VinagreVncTab *tab, gboolean active);
gboolean	vinagre_vnc_tab_get_scaling		(VinagreVncTab *tab);
void		vinagre_vnc_tab_set_viewonly		(VinagreVncTab *tab, gboolean active);
gboolean	vinagre_vnc_tab_get_viewonly		(VinagreVncTab *tab);

gint		vinagre_vnc_tab_get_original_width	(VinagreVncTab *tab);
gint		vinagre_vnc_tab_get_original_height	(VinagreVncTab *tab);
void		vinagre_vnc_tab_original_size		(VinagreVncTab *tab);

gboolean	vinagre_vnc_tab_is_pointer_grab		(VinagreVncTab *tab);
G_END_DECLS

#endif  /* __VINAGRE_VNC_TAB_H__  */
/* vim: set ts=8: */
