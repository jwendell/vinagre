/*
 * vinagre-tab.h
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

#ifndef __VINAGRE_TAB_H__
#define __VINAGRE_TAB_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_TAB              (vinagre_tab_get_type())
#define VINAGRE_TAB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_TAB, VinagreTab))
#define VINAGRE_TAB_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_TAB, VinagreTab const))
#define VINAGRE_TAB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_TAB, VinagreTabClass))
#define VINAGRE_IS_TAB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_TAB))
#define VINAGRE_IS_TAB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_TAB))
#define VINAGRE_TAB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_TAB, VinagreTabClass))

typedef struct _VinagreTabPrivate VinagreTabPrivate;
typedef struct _VinagreTab        VinagreTab;
typedef struct _VinagreTabClass   VinagreTabClass;

#include "vinagre-connection.h"
#include "vinagre-notebook.h"
#include "vinagre-window.h"

struct _VinagreTab 
{
  GtkVBox vbox;
  VinagreTabPrivate *priv;
};

struct _VinagreTabClass 
{
  GtkVBoxClass parent_class;

  /* Signals */
  void		(* tab_connected)		(VinagreTab *tab);
  void		(* tab_disconnected)		(VinagreTab *tab);
  void		(* tab_initialized)		(VinagreTab *tab);
};

GType 		  vinagre_tab_get_type		(void) G_GNUC_CONST;

GtkWidget 	  *vinagre_tab_new 		(VinagreConnection *conn,
						 VinagreWindow     *window);

VinagreConnection *vinagre_tab_get_conn		(VinagreTab *tab);

GtkWidget         *vinagre_tab_get_vnc		(VinagreTab *tab);

gchar             *vinagre_tab_get_tooltips     (VinagreTab *tab);

void		  vinagre_tab_set_title		(VinagreTab *tab,
						 const char *title);

void		  vinagre_tab_set_notebook	(VinagreTab *tab,
						 VinagreNotebook *nb);
VinagreNotebook   *vinagre_tab_get_notebook	(VinagreTab *tab);

void		  vinagre_tab_take_screenshot	(VinagreTab *tab);
G_END_DECLS

#endif  /* __VINAGRE_TAB_H__  */
