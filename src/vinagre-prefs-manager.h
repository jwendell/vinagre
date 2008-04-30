/*
 * vinagre-prefs-manager.h
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008  Jonh Wendell <wendell@bani.com.br>
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

#include <glib.h>

typedef struct _VinagrePrefsManager VinagrePrefsManager;


#ifndef __VINAGRE_PREFS_MANAGER_H__
#define __VINAGRE_PREFS_MANAGER_H__

#define VINAGRE_BASE_KEY		"/apps/vinagre"

#define VPM_TOOLBAR_VISIBLE	 	VINAGRE_BASE_KEY "/toolbar_visible"
#define VPM_STATUSBAR_VISIBLE		VINAGRE_BASE_KEY "/statusbar_visible"
#define VPM_SIDE_PANE_VISIBLE		VINAGRE_BASE_KEY "/side_pane_visible"

#define VPM_WINDOW_STATE		VINAGRE_BASE_KEY "/window_state"
#define VPM_WINDOW_WIDTH		VINAGRE_BASE_KEY "/window_width"
#define VPM_WINDOW_HEIGHT		VINAGRE_BASE_KEY "/window_height"
#define VPM_SIDE_PANEL_SIZE		VINAGRE_BASE_KEY "/side_panel_size"

#define VPM_SHARED_FLAG			VINAGRE_BASE_KEY "/shared_flag"

/* Fallback default values. Keep in sync with vinagre.schemas */

#define VPM_DEFAULT_TOOLBAR_VISIBLE	1 /* TRUE */
#define VPM_DEFAULT_STATUSBAR_VISIBLE	1 /* TRUE */
#define VPM_DEFAULT_SIDE_PANE_VISIBLE	1 /* TRUE */

#define VPM_DEFAULT_WINDOW_STATE	0
#define VPM_DEFAULT_WINDOW_WIDTH	650
#define VPM_DEFAULT_WINDOW_HEIGHT	500
#define VPM_DEFAULT_SIDE_PANEL_SIZE	200

#define VPM_DEFAULT_SHARED_FLAG		1 /* TRUE */

/** LIFE CYCLE MANAGEMENT FUNCTIONS **/

gboolean		 vinagre_prefs_manager_init (void);

/* This function must be called before exiting vinagre */
void			 vinagre_prefs_manager_shutdown (void);


/** PREFS MANAGEMENT FUNCTIONS **/

/* Toolbar visible */
gboolean		 vinagre_prefs_manager_get_toolbar_visible	(void);
void			 vinagre_prefs_manager_set_toolbar_visible	(gboolean tv);

/* Statusbar visible */
gboolean		 vinagre_prefs_manager_get_statusbar_visible	(void);
void			 vinagre_prefs_manager_set_statusbar_visible	(gboolean sv);

/* Side pane visible */
gboolean		 vinagre_prefs_manager_get_side_pane_visible	(void);
void			 vinagre_prefs_manager_set_side_pane_visible	(gboolean tv);

/* Window state */
gint		 vinagre_prefs_manager_get_window_state		(void);
void 		 vinagre_prefs_manager_set_window_state		(gint ws);

/* Window size */
void		 vinagre_prefs_manager_get_window_size		(gint *width,
								 gint *height);
void 		 vinagre_prefs_manager_set_window_size		(gint width,
								 gint height);

/* Side panel */
gint	 	 vinagre_prefs_manager_get_side_panel_size	(void);
void 		 vinagre_prefs_manager_set_side_panel_size	(gint ps);

/* Shared flag */
gboolean	vinagre_prefs_manager_get_shared_flag		(void);
void		vinagre_prefs_manager_set_shared_flag		(gboolean sf);

#endif  /* __VINAGRE_PREFS_MANAGER_H__ */

/* vim: ts=8 */
