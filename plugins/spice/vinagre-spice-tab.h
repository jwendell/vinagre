/*
 * vinagre-spice-tab.h
 * SPICE Tab
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

#ifndef __VINAGRE_SPICE_TAB_H__
#define __VINAGRE_SPICE_TAB_H__

#include <vinagre/vinagre-tab.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_SPICE_TAB              (vinagre_spice_tab_get_type())
#define VINAGRE_SPICE_TAB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_SPICE_TAB, VinagreSpiceTab))
#define VINAGRE_SPICE_TAB_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_SPICE_TAB, VinagreSpiceTab const))
#define VINAGRE_SPICE_TAB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_SPICE_TAB, VinagreSpiceTabClass))
#define VINAGRE_IS_SPICE_TAB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_SPICE_TAB))
#define VINAGRE_IS_SPICE_TAB_CLASS(klass)...(G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_SPICE_TAB))
#define VINAGRE_SPICE_TAB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_SPICE_TAB, VinagreSpiceTabClass))

typedef struct _VinagreSpiceTabPrivate VinagreSpiceTabPrivate;
typedef struct _VinagreSpiceTab        VinagreSpiceTab;
typedef struct _VinagreSpiceTabClass   VinagreSpiceTabClass;


struct _VinagreSpiceTab
{
  VinagreTab tab;
  VinagreSpiceTabPrivate *priv;
};

struct _VinagreSpiceTabClass
{
  VinagreTabClass parent_class;
};

GType		vinagre_spice_tab_get_type		(void) G_GNUC_CONST;

GtkWidget *	vinagre_spice_tab_new			(VinagreConnection *conn,
							 VinagreWindow	   *window);

void		vinagre_spice_tab_send_ctrlaltdel	(VinagreSpiceTab *tab);
void		vinagre_spice_tab_paste_text		(VinagreSpiceTab *tab,
							 const gchar   *text);

gboolean	vinagre_spice_tab_set_scaling		(VinagreSpiceTab *tab, gboolean active);
gboolean	vinagre_spice_tab_get_scaling		(VinagreSpiceTab *tab);
gboolean	vinagre_spice_tab_set_resize_guest	(VinagreSpiceTab *tab, gboolean active);
gboolean	vinagre_spice_tab_get_resize_guest	(VinagreSpiceTab *tab);
void		vinagre_spice_tab_set_viewonly		(VinagreSpiceTab *tab, gboolean active);
gboolean	vinagre_spice_tab_get_viewonly		(VinagreSpiceTab *tab);
void		vinagre_spice_tab_set_auto_clipboard	(VinagreSpiceTab *tab, gboolean active);
gboolean	vinagre_spice_tab_get_auto_clipboard	(VinagreSpiceTab *tab);

gboolean	vinagre_spice_tab_is_mouse_grab		(VinagreSpiceTab *tab);
G_END_DECLS

#endif  /* __VINAGRE_SPICE_TAB_H__  */
/* vim: set ts=8: */
