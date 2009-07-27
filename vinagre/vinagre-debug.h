/*
 * vinagre-debug.h
 * This file is part of vinagre
 *
 * Based on gedit code
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002 - 2005 Paolo Maggi
 *
 * vinagre-debug.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-debug.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_DEBUG_H__
#define __VINAGRE_DEBUG_H__

#include <glib.h>

/*
 * Set an environmental var of the same name to turn on
 * debugging output. Setting VINAGRE_DEBUG will turn on all
 * sections.
 */
typedef enum {
	VINAGRE_NO_DEBUG       = 0,
	VINAGRE_DEBUG_VIEW     = 1 << 0,
	VINAGRE_DEBUG_PRINT    = 1 << 1,
	VINAGRE_DEBUG_PREFS    = 1 << 2,
	VINAGRE_DEBUG_PLUGINS  = 1 << 3,
	VINAGRE_DEBUG_UTILS    = 1 << 4,
	VINAGRE_DEBUG_WINDOW   = 1 << 5,
	VINAGRE_DEBUG_LOADER   = 1 << 6,
	VINAGRE_DEBUG_APP      = 1 << 7,
	VINAGRE_DEBUG_TUBE     = 1 << 8
} VinagreDebugSection;


#define	DEBUG_VIEW	VINAGRE_DEBUG_VIEW,    __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_PRINT	VINAGRE_DEBUG_PRINT,   __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_PREFS	VINAGRE_DEBUG_PREFS,   __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_PLUGINS	VINAGRE_DEBUG_PLUGINS, __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_UTILS	VINAGRE_DEBUG_UTILS,   __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_WINDOW	VINAGRE_DEBUG_WINDOW,  __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_LOADER	VINAGRE_DEBUG_LOADER,  __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_APP	VINAGRE_DEBUG_APP,     __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_TUBE	VINAGRE_DEBUG_TUBE,    __FILE__, __LINE__, G_STRFUNC

void vinagre_debug_init (void);

void vinagre_debug (VinagreDebugSection  section,
		    const gchar         *file,
		    gint                 line,
		    const gchar         *function);

void vinagre_debug_message (VinagreDebugSection  section,
			    const gchar         *file,
			    gint                 line,
			    const gchar         *function,
			    const gchar         *format, ...) G_GNUC_PRINTF(5, 6);


#endif /* __VINAGRE_DEBUG_H__ */
/* vim: set ts=8: */
