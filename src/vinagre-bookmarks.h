/*
 * vinagre-bookmarks.h
 * This file is part of vinagre
 *
 * Copyright (C) 2007  Jonh Wendell <wendell@bani.com.br>
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

#ifndef __VINAGRE_FAVORITES_H__
#define __VINAGRE_FAVORITES_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "vinagre-connection.h"
#include "vinagre-window.h"

void			vinagre_bookmarks_init		(void);
void			vinagre_bookmarks_finalize	(void);

gboolean		vinagre_bookmarks_add		(VinagreConnection *conn,
							 VinagreWindow     *window);
gboolean		vinagre_bookmarks_del		(VinagreConnection *conn,
							 VinagreWindow     *window);
gboolean		vinagre_bookmarks_edit		(VinagreConnection *conn,
							 VinagreWindow     *window);

GList			*vinagre_bookmarks_get_all	(void);
VinagreConnection	*vinagre_bookmarks_exists	(const char *host, int port);

#endif  /* __VINAGRE_FAVORITES_H__ */
/* vim: ts=8 */
