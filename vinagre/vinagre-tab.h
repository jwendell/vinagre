/*
 * vinagre-tab.h
 * Abstract base class for all types of tabs: VNC, RDP, etc.
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008,2009 - Jonh Wendell <wendell@bani.com.br>
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
#define VINAGRE_TAB_KEY               "VINAGRE_TAB_KEY"

typedef struct _VinagreTabPrivate VinagreTabPrivate;
typedef struct _VinagreTab        VinagreTab;
typedef struct _VinagreTabClass   VinagreTabClass;

#include "vinagre-connection.h"
#include "vinagre-notebook.h"
#include "vinagre-window.h"

typedef enum
{
  VINAGRE_TAB_STATE_INITIALIZING = 1,
  VINAGRE_TAB_STATE_CONNECTED,
  VINAGRE_TAB_STATE_INVALID
} VinagreTabState;


struct _VinagreTab 
{
  GtkBox box;
  VinagreTabPrivate *priv;
};

typedef struct
{
  gchar     **paths; /* NULL-terminated array of strings */
  GtkAction *action;
} VinagreTabUiAction;

struct _VinagreTabClass 
{
  GtkBoxClass parent_class;

  /* Signals */
  void		(* tab_connected)			(VinagreTab *tab);
  void		(* tab_disconnected)			(VinagreTab *tab);
  void		(* tab_initialized)			(VinagreTab *tab);
  void		(* tab_auth_failed)			(VinagreTab *tab, const gchar *msg);

  /* Virtual functions */
  void		(* impl_get_dimensions)			(VinagreTab *tab, int *w, int *h);
  const GSList *(* impl_get_always_sensitive_actions)	(VinagreTab *tab);
  const GSList *(* impl_get_connected_actions)		(VinagreTab *tab);
  const GSList *(* impl_get_initialized_actions)	(VinagreTab *tab);
  gchar *	(* impl_get_extra_title)		(VinagreTab *tab);

  /* Abstract functions */
  gchar *	(* impl_get_tooltip)			(VinagreTab *tab);
  GdkPixbuf *	(* impl_get_screenshot)			(VinagreTab *tab);
};

GType			vinagre_tab_get_type		(void) G_GNUC_CONST;

GtkWidget *		vinagre_tab_new 		(VinagreConnection *conn,
							 VinagreWindow     *window);

VinagreConnection *	vinagre_tab_get_conn		(VinagreTab *tab);
VinagreWindow *		vinagre_tab_get_window		(VinagreTab *tab);

void			vinagre_tab_add_view		(VinagreTab *tab, GtkWidget *view);
GtkWidget *		vinagre_tab_get_view		(VinagreTab *tab);

void			vinagre_tab_set_title		(VinagreTab *tab,
							 const char *title);

void			vinagre_tab_set_notebook	(VinagreTab *tab,
							 VinagreNotebook *nb);
VinagreNotebook *	vinagre_tab_get_notebook	(VinagreTab *tab);

VinagreTabState		vinagre_tab_get_state		(VinagreTab *tab);
VinagreTab *		vinagre_tab_get_from_connection	(VinagreConnection *conn);

void			vinagre_tab_set_has_screenshot	(VinagreTab *tab, gboolean has_screenshot);
gboolean		vinagre_tab_get_has_screenshot	(VinagreTab *tab);
void			vinagre_tab_take_screenshot	(VinagreTab *tab);

gchar *			vinagre_tab_get_tooltip		(VinagreTab *tab);
void			vinagre_tab_get_dimensions	(VinagreTab *tab, int *w, int *h);

const GSList *		vinagre_tab_get_always_sensitive_actions(VinagreTab *tab);
const GSList *		vinagre_tab_get_connected_actions	(VinagreTab *tab);
const GSList *		vinagre_tab_get_initialized_actions	(VinagreTab *tab);

gchar *			vinagre_tab_get_extra_title	(VinagreTab *tab);
GtkWidget *		vinagre_tab_get_toolbar		(VinagreTab *tab);

void			vinagre_tab_free_actions	(GSList *actions);
const gchar		*vinagre_tab_get_icon_name	(VinagreTab *tab);

/* Protected functions */
void			vinagre_tab_set_save_credentials	(VinagreTab *tab, gboolean value);
void			vinagre_tab_save_credentials_in_keyring (VinagreTab *tab);
gboolean		vinagre_tab_find_credentials_in_keyring	(VinagreTab *tab,
								 gchar **username,
								 gchar **password);
void			vinagre_tab_remove_credentials_from_keyring (VinagreTab *tab);

void			vinagre_tab_remove_from_notebook	(VinagreTab *tab);
void			vinagre_tab_add_recent_used		(VinagreTab *tab);
void			vinagre_tab_set_state			(VinagreTab *tab,
								 VinagreTabState state);

void			vinagre_tab_add_actions			(VinagreTab *tab,
								 const GtkActionEntry *entries,
								 guint n_entries);
void			vinagre_tab_add_toggle_actions		(VinagreTab *tab,
								 const GtkToggleActionEntry *entries,
								 guint n_entries);
G_END_DECLS

#endif  /* __VINAGRE_TAB_H__  */
/* vim: set ts=8: */
