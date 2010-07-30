/*
 * vinagre-protocol-ext.h
 * This file is part of vinagre
 *
 * Copyright (C) 2010 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-protocol.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-protocol.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_PROTOCOL_EXT_H__
#define __VINAGRE_PROTOCOL_EXT_H__

#include <glib-object.h>
#include <libpeas/peas.h>
#include "vinagre-window.h"

typedef PeasExtension VinagreProtocolExt;

G_BEGIN_DECLS

void		    vinagre_protocol_ext_parse_mdns_dialog	  (VinagreProtocolExt *protocol,
								   GtkWidget          *connect_widget,
								   GtkWidget          *dialog);
GSList *	    vinagre_protocol_ext_get_context_groups	  (VinagreProtocolExt *protocol);
const gchar *       vinagre_protocol_ext_get_protocol		  (VinagreProtocolExt *protocol);
gchar **	    vinagre_protocol_ext_get_public_description	  (VinagreProtocolExt *protocol);
gint		    vinagre_protocol_ext_get_default_port	  (VinagreProtocolExt *protocol);
VinagreConnection * vinagre_protocol_ext_new_connection		  (VinagreProtocolExt *protocol);
VinagreConnection * vinagre_protocol_ext_new_connection_from_file (VinagreProtocolExt *protocol,
								   const gchar        *data,
								   gboolean           use_bookmarks,
								   gchar              **error_msg);
const gchar *	    vinagre_protocol_ext_get_mdns_service	  (VinagreProtocolExt *protocol);
GtkWidget *	    vinagre_protocol_ext_new_tab		  (VinagreProtocolExt   *protocol,
								   VinagreConnection *conn,
								   VinagreWindow     *window);
GtkWidget *	    vinagre_protocol_ext_get_connect_widget	  (VinagreProtocolExt   *protocol,
								   VinagreConnection *initial_settings);
GtkFileFilter *	    vinagre_protocol_ext_get_file_filter	  (VinagreProtocolExt *protocol);

GdkPixbuf *	    vinagre_protocol_ext_get_icon		  (VinagreProtocolExt *protocol,
								   gint               size);
const gchar *	    vinagre_protocol_ext_get_icon_name		  (VinagreProtocolExt *protocol);

G_END_DECLS

#endif  /* __VINAGRE_PROTOCOL_EXT_H__ */

/* vim: set ts=8: */
