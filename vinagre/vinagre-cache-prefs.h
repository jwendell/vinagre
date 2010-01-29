/*
 * vinagre-cache-prefs.h
 * This file is part of vinagre
 *
 * Copyright (C) Jonh Wendell 2010 <wendell@bani.com.br>
 *
 * vinagre-prefs.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * vinagre-prefs.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _VINAGRE_CACHE_PREFS_H_
#define _VINAGRE_CACHE_PREFS_H_

G_BEGIN_DECLS

#include <glib.h>

void vinagre_cache_prefs_init (void);
void vinagre_cache_prefs_finalize (void);

gboolean vinagre_cache_prefs_get_boolean (const gchar *group, const gchar *key, gboolean default_value);
void     vinagre_cache_prefs_set_boolean (const gchar *group, const gchar *key, gboolean value);

gchar *  vinagre_cache_prefs_get_string (const gchar *group, const gchar *key, const gchar *default_value);
void     vinagre_cache_prefs_set_string (const gchar *group, const gchar *key, const gchar *value);

gint     vinagre_cache_prefs_get_integer (const gchar *group, const gchar *key, gint default_value);
void     vinagre_cache_prefs_set_integer (const gchar *group, const gchar *key, gint value);

G_END_DECLS

#endif /* _VINAGRE_CACHE_PREFS_H_ */
