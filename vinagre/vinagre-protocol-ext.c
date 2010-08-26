/*
 * vinagre-protocol-ext.c
 * This file is part of vinagre
 *
 * Copyright (C) 2010 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-protocol.c is free software: you can redistribute it and/or modify it
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

#include "vinagre-protocol-ext.h"

const gchar *
vinagre_protocol_ext_get_protocol (VinagreProtocolExt *protocol)
{
  const gchar *result = NULL;

//  g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "get_protocol", &result);

  return result;
}

gchar **
vinagre_protocol_ext_get_public_description (VinagreProtocolExt *protocol)
{
  gchar **result = NULL;

//  g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "get_public_description", &result);

  return result;
}

gint
vinagre_protocol_ext_get_default_port (VinagreProtocolExt *protocol)
{
  gint result = 0;

//  g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "get_default_port", &result);

  return result;
}

const gchar *
vinagre_protocol_ext_get_mdns_service (VinagreProtocolExt *protocol)
{
  const gchar *result = NULL;

//  g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "get_mdns_service", &result);

  return result;
}

/**
 * vinagre_protocol_ext_get_context_groups: blah
 * @protocol: A protocol
 *
 * Return value: (element-type any) (transfer container): blah
 */
GSList *
vinagre_protocol_ext_get_context_groups (VinagreProtocolExt *protocol)
{
  GSList *result = NULL;

//  g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "get_context_groups", &result);

  return result;
}

GtkFileFilter *
vinagre_protocol_ext_get_file_filter (VinagreProtocolExt *protocol)
{
  GtkFileFilter *result = NULL;

//  g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "get_file_filter", &result);

  return result;
}

GtkWidget *
vinagre_protocol_ext_new_tab (VinagreProtocolExt *protocol,
			      VinagreConnection  *conn,
			      VinagreWindow      *window)
{
  GtkWidget *result = NULL;

//  g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "new_tab", conn, window, &result);

  return result;
}

VinagreConnection *
vinagre_protocol_ext_new_connection (VinagreProtocolExt *protocol)
{
  VinagreConnection *result = NULL;

  //g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "new_connection", &result);

  return result;
}

VinagreConnection *
vinagre_protocol_ext_new_connection_from_file (VinagreProtocolExt *protocol,
					       const gchar        *data,
					       gboolean           use_bookmarks,
					       gchar              **error_msg)
{
  VinagreConnection *result = NULL;

  //g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "new_connection_from_file", data, use_bookmarks, error_msg, &result);

  return result;
}

/**
 * vinagre_protocol_ext_get_connect_widget:
 *
 * @protocol: a protocol
 * @initial_settings: (allow-none): bla bla
 * @returns: (allow-none): a widget
 */
GtkWidget *
vinagre_protocol_ext_get_connect_widget (VinagreProtocolExt *protocol,
					 VinagreConnection  *initial_settings)
{
  GtkWidget *result = NULL;

  //g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "get_connect_widget", initial_settings, &result);

  return result;
}

void
vinagre_protocol_ext_parse_mdns_dialog (VinagreProtocolExt *protocol,
					GtkWidget          *connect_widget,
					GtkWidget          *dialog)
{
  //g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "parse_mdns_dialog", connect_widget, dialog);
}

const gchar *
vinagre_protocol_ext_get_icon_name (VinagreProtocolExt *protocol)
{
  const gchar *result = NULL;

  //g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "get_icon_name", &result);

  return result;
}

GdkPixbuf *
vinagre_protocol_ext_get_icon (VinagreProtocolExt *protocol,
			       gint                size)
{
  GdkPixbuf *result = NULL;

  //g_return_if_fail (peas_extension_get_type (protocol) != VINAGRE_TYPE_PROTOCOL);

  peas_extension_call (protocol, "get_icon", size, &result);

  return result;
}

/* vim: set ts=8: */
