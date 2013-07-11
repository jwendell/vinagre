/*
 * vinagre-protocol.h
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

#ifndef __VINAGRE_PROTOCOL_H__
#define __VINAGRE_PROTOCOL_H__

#include <glib-object.h>

#include "vinagre-window.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_PROTOCOL              (vinagre_protocol_get_type())
#define VINAGRE_PROTOCOL(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_PROTOCOL, VinagreProtocol))
#define VINAGRE_IS_PROTOCOL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_PROTOCOL))
#define VINAGRE_PROTOCOL_GET_IFACE(obj)    (G_TYPE_INSTANCE_GET_INTERFACE((obj), VINAGRE_TYPE_PROTOCOL, VinagreProtocolInterface))

typedef struct _VinagreProtocol VinagreProtocol;
typedef struct _VinagreProtocolInterface VinagreProtocolInterface;

struct _VinagreProtocolInterface
{
  GTypeInterface g_iface;

  /* Virtual public methods */
  const gchar		*(*get_protocol)		(VinagreProtocol *protocol);
  gchar			**(*get_public_description)	(VinagreProtocol *protocol);
  gint			(*get_default_port)		(VinagreProtocol *protocol);
  VinagreConnection	*(*new_connection)		(VinagreProtocol *protocol);
  VinagreConnection	*(*new_connection_from_file)	(VinagreProtocol *protocol,
							 const gchar     *data,
							 gboolean         use_bookmarks,
							 gchar          **error_msg);
  gboolean              (*recognize_file)               (VinagreProtocol *protocol,
                                                         GFile           *file);
  const gchar		*(*get_mdns_service)		(VinagreProtocol *protocol);
  GtkWidget 		*(*new_tab)			(VinagreProtocol   *protocol,
							 VinagreConnection *conn,
							 VinagreWindow     *window);
  GtkWidget 		*(*get_connect_widget)		(VinagreProtocol   *protocol,
							 VinagreConnection *initial_settings);
  void			(*parse_mdns_dialog)		(VinagreProtocol *protocol,
							 GtkWidget       *connect_widget,
							 GtkWidget       *dialog);
  GtkFileFilter		*(*get_file_filter)		(VinagreProtocol *protocol);
  GdkPixbuf		*(*get_icon)			(VinagreProtocol *protocol,
							 gint             size);
  const gchar		*(*get_icon_name)		(VinagreProtocol *protocol);
  GSList		*(*get_context_groups)		(VinagreProtocol *protocol);
};

/*
 * Public methods
 */
GType			vinagre_protocol_get_type		  (void) G_GNUC_CONST;

void			vinagre_protocol_parse_mdns_dialog	  (VinagreProtocol *protocol,
								   GtkWidget *connect_widget,
								   GtkWidget *dialog);
const gchar *		vinagre_protocol_get_protocol		  (VinagreProtocol *protocol);
gchar **		vinagre_protocol_get_public_description	  (VinagreProtocol *protocol);
gint			vinagre_protocol_get_default_port	  (VinagreProtocol *protocol);
VinagreConnection *	vinagre_protocol_new_connection		  (VinagreProtocol *protocol);
VinagreConnection *	vinagre_protocol_new_connection_from_file (VinagreProtocol *protocol,
								   const gchar     *data,
								   gboolean         use_bookmarks,
								   gchar           **error_msg);
gboolean                vinagre_protocol_recognize_file           (VinagreProtocol *protocol,
                                                                   GFile           *file);
const gchar *		vinagre_protocol_get_mdns_service	  (VinagreProtocol *protocol);

GtkWidget *		vinagre_protocol_new_tab		  (VinagreProtocol   *protocol,
								   VinagreConnection *conn,
								   VinagreWindow     *window);
GtkWidget *		vinagre_protocol_get_connect_widget	  (VinagreProtocol   *protocol,
								   VinagreConnection *initial_settings);
GtkFileFilter *		vinagre_protocol_get_file_filter	  (VinagreProtocol *protocol);

GdkPixbuf *		vinagre_protocol_get_icon		  (VinagreProtocol *protocol,
								   gint             size);
const gchar *		vinagre_protocol_get_icon_name		  (VinagreProtocol *protocol);
GSList *		vinagre_protocol_get_context_groups	  (VinagreProtocol *protocol);

G_END_DECLS

#endif  /* __VINAGRE_PROTOCOL_H__ */
/* vim: set ts=8: */
